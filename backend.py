from dataclasses import dataclass
import json
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
import os
import glob
import importlib.util
import asyncio
import uvicorn

from algo import Algo
from config import ALGO_CONTROLLER_PORT
from market_types import Trade
from algo_builder import build_algo_file
from websockets_accessor import WebsocketsAccessor


app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # In production, replace with specific origin
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

@dataclass
class RunningAlgo:
    algo_class: type[Algo]
    path: str
    module_name: str
    algo_instance: Algo
    state: str  # running, pending, error, stopped
    actions: list[str]

running_algos: list[RunningAlgo] = []

@app.get('/api/algos')
async def get_algos():
    return [{
        'module_name': algo.module_name,
        'state': algo.state,
        'actions': algo.actions[-20:]  # Return only last 20 actions
    } for algo in running_algos]

@app.post('/api/algos/{module_name}/stop')
async def stop_algo(module_name: str):
    for algo in running_algos:
        if algo.module_name == module_name:
            algo.state = 'stopped'
            return {'status': 'success'}
    raise HTTPException(status_code=404, detail='Algo not found')

@app.post('/api/algos/{module_name}/restart')
async def restart_algo(module_name: str):
    for algo in running_algos:
        if algo.module_name == module_name:
            algo.state = 'running'
            return {'status': 'success'}
    raise HTTPException(status_code=404, detail='Algo not found')

@app.post('/api/algos')
async def create_algo(body: dict):
    print(f'create algo {body=}')
    try:
        algo_title: str = body['title']
        module_name = '_'.join(algo_title.lower().split(' '))
        file_path = f'generated_algos/{module_name}.py'
        print(f'create algo {algo_title=} {module_name=}')

        build_algo_file(body, file_path)
        running_algo = load_algo_from_file(module_name, file_path)

        config_path = f'algo_configs/{module_name}.json'
        with open(config_path, 'w') as config_file:
            json.dump({
                'algo_title': algo_title,
                'module_name': module_name,
                'file_path': file_path,
                'state': running_algo.state,
                'schema': body,
            }, config_file, indent=4)

        running_algos[:] = [algo for algo in running_algos if algo.module_name != module_name]
        running_algos.append(running_algo)

        return {"status": "success"}
    except Exception as e:
        raise HTTPException(status_code=400, detail=str(e))

def on_trade(trade: Trade):
    for algo in running_algos:
        if algo.state != "running":
            continue
        action = algo.algo_instance.on_trade(trade)
        if action:
            algo.actions.append(action)

def load_existing_algos():
    os.makedirs('generated_algos', exist_ok=True)
    for file_path in glob.glob('generated_algos/*.py'):
        module_name = os.path.splitext(os.path.basename(file_path))[0]
        try:
            running_algo = load_algo_from_file(module_name, file_path)
            running_algos.append(running_algo)
            print(f'Loaded algorithm: {module_name}')
        except Exception as e:
            print(f'Error loading algorithm {file_path}: {str(e)}')


def load_algo_from_file(module_name: str, file_path: str) -> RunningAlgo:
    """Load an algorithm from a file, auto-discover the Algo subclass, and create a RunningAlgo instance."""
    spec = importlib.util.spec_from_file_location(module_name, file_path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)

    # Find all classes in the module that inherit from Algo
    algo_classes = [
        cls for name, cls in vars(module).items()
        if isinstance(cls, type) and issubclass(cls, Algo) and cls is not Algo
    ]
    if len(algo_classes) == 0:
        raise RuntimeError(f"No Algo subclass found in {file_path}")
    if len(algo_classes) > 1:
        raise RuntimeError(f"Multiple Algo subclasses found in {file_path}: {[cls.__name__ for cls in algo_classes]}")
    algo_class = algo_classes[0]
    algo_instance = algo_class()

    # Create and return RunningAlgo instance
    return RunningAlgo(
        algo_class=algo_class,
        path=file_path,
        module_name=module_name,
        algo_instance=algo_instance,
        state='running',
        actions=[]
    )


class AlgoServer(uvicorn.Server):
    def __init__(self, config: uvicorn.Config):
        super().__init__(config)
        self.config = config
        self.config.log_level = 'debug'

    async def serve(self):
        self.config.setup_event_loop()
        return await super().serve()

async def main():
    load_existing_algos()

    config = uvicorn.Config(app, host="0.0.0.0", port=ALGO_CONTROLLER_PORT)
    server = AlgoServer(config)

    await asyncio.gather(
        WebsocketsAccessor().run(on_trade),
        server.serve()
    )

if __name__ == "__main__":
    asyncio.run(main())

