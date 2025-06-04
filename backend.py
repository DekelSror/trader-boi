"""
this here backend simulates the client end of a direct connection to a stock exchange
"""

from dataclasses import dataclass
import json
import socket
import pickle
import importlib.util

from aggregations.ohlc import OHLCAggregator
from algo import Algo
from db import TimeseriesDB
from db.memory_impl import MemoryTimeseries
from market_types import Trade
from mem_agg_man import add_to_agg
from file_watcher import FileWatcher



aggs = [OHLCAggregator('AAA', 300, lambda c: add_to_agg('candles_300_AAA', c))]


@dataclass
class RunningAlgo:
    algo_class: type[Algo]
    path: str
    module_name: str
    algo_instance: Algo
    
running_algos: list[RunningAlgo] = []



host='127.0.0.1'
port=9000
timeseries_db = TimeseriesDB(MemoryTimeseries)
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

print(f"Connecting to server at {host}:{port}...")
client_socket.connect((host, port))
print(f"Connected to server")


def on_algos_modified() -> Algo:
    global running_algos
    with open('algos.json') as alg_file:
        try:
            file_algos = json.loads(alg_file.read())
            for algo in file_algos:
                if algo['name'] in [ra.module_name for ra in running_algos]:
                    # TODO: path can change, or changes were made to code
                    # and it has to be reloaded or re-instantiated
                    continue
                AlgorithmClass = import_algorithm_from_path(algo['path'], algo['name'])
                running_algos.append(RunningAlgo(
                    algo_class=AlgorithmClass,
                    path=algo['path'],
                    algo_instance=AlgorithmClass(),
                    module_name=algo['name']
                ))
        except json.JSONDecodeError as e:
            # file is being edited
            return


alg_watch = FileWatcher('algos.json', on_algos_modified)


def import_algorithm_from_path(path: str, module_name: str, class_name: str = 'Algorithm'):
    spec = importlib.util.spec_from_file_location(module_name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return getattr(module, class_name)


while True:
    alg_watch.check()
    raw_trade = client_socket.recv(4096)
    trade: Trade = pickle.loads(raw_trade)
    point = (trade.timestamp * 1000000000, trade.price)
    ts = timeseries_db.get_series(trade.symbol)
    ts.write_points(point)
    for agg in aggs:
        agg.on_trade(trade)
    for algo in running_algos:
        algo.algo_instance.on_trade(trade)
