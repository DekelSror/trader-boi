"""
this here backend simulates the client end of a direct connection to a stock exchange
"""

import socket
import pickle

from aggregations.ohlc import OHLCAggregator
from algo import Algo
from algorithms.simple_ma_crossover import SimpleMACrossoverAlgo
from db import TimeseriesDB
from db.memory_impl import MemoryTimeseries
from market_types import OHLC, Trade
from mem_agg_man import add_to_agg



aggs = [OHLCAggregator('AAA', 300, lambda c: add_to_agg('candles_300_AAA', c))]
algs: list[Algo] = [SimpleMACrossoverAlgo()]



host='127.0.0.1'
port=9000
timeseries_db = TimeseriesDB(MemoryTimeseries)
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

print(f"Connecting to server at {host}:{port}...")
client_socket.connect((host, port))
print(f"Connected to server")

while True:
    raw_trade = client_socket.recv(4096)
    trade: Trade = pickle.loads(raw_trade)
    point = (trade.timestamp * 1000000000, trade.price)
    ts = timeseries_db.get_series(trade.symbol)
    ts.write_points(point)
    for agg in aggs:
        agg.on_trade(trade)
    for alg in algs:
        alg.on_trade(trade)
