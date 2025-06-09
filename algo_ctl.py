from dataclasses import dataclass
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
import websockets
import json
import os
import glob
import importlib.util
import asyncio
import uvicorn

from algo import Algo
from config import TRADE_HUB_WS_URL, ALGO_CONTROLLER_PORT
from market_types import Trade
from algo_builder import build_algo_file


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

@app.get("/api/algos")
async def get_algos():
    return [{
        "module_name": algo.module_name,
        "state": algo.state,
        "actions": algo.actions[-20:]  # Return only last 20 actions
    } for algo in running_algos]

@app.post("/api/algos/{module_name}/stop")
async def stop_algo(module_name: str):
    for algo in running_algos:
        if algo.module_name == module_name:
            algo.state = "stopped"
            return {"status": "success"}
    raise HTTPException(status_code=404, detail="Algo not found")

@app.post("/api/algos/{module_name}/restart")
async def restart_algo(module_name: str):
    for algo in running_algos:
        if algo.module_name == module_name:
            algo.state = "running"
            return {"status": "success"}
    raise HTTPException(status_code=404, detail="Algo not found")

def load_algo_from_file(module_name: str, file_path: str) -> RunningAlgo:
    """Load an algorithm from a file and create a RunningAlgo instance"""
    # Import the module
    spec = importlib.util.spec_from_file_location(module_name, file_path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    
    # Create algorithm instance
    algo_class = getattr(module, "Algorithm")
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

@app.post('/api/algos')
async def create_algo(config: dict):
    try:
        module_name = config['import_params']['module_name']
        file_path = f"generated_algos/{module_name}.py"
        
        # Build the algorithm file
        build_algo_file(config, file_path)
        
        # Load the algorithm
        running_algo = load_algo_from_file(module_name, file_path)
        
        # Remove any existing algo with same name
        running_algos[:] = [algo for algo in running_algos if algo.module_name != module_name]
        running_algos.append(running_algo)
        
        return {"status": "success"}
    except Exception as e:
        raise HTTPException(status_code=400, detail=str(e))

async def process_hub_trades():
    async with websockets.connect(TRADE_HUB_WS_URL) as hub_ws:
        while True:
            trade_data = await hub_ws.recv()
            trade: Trade = Trade(**json.loads(trade_data))
            for algo in running_algos:
                if algo.state != "running":
                    continue
                action = algo.algo_instance.on_trade(trade)
                if action:
                    algo.actions.append(action)

def load_existing_algos():
    os.makedirs("generated_algos", exist_ok=True)
    for file_path in glob.glob("generated_algos/*.py"):
        module_name = os.path.splitext(os.path.basename(file_path))[0]
        try:
            running_algo = load_algo_from_file(module_name, file_path)
            running_algos.append(running_algo)
            print(f"Loaded algorithm: {module_name}")
        except Exception as e:
            print(f"Error loading algorithm {file_path}: {str(e)}")

class AlgoServer(uvicorn.Server):
    def __init__(self, config: uvicorn.Config):
        super().__init__(config)
        self.config = config

    async def serve(self):
        self.config.setup_event_loop()
        return await super().serve()

async def main():
    load_existing_algos()
    
    config = uvicorn.Config(app, host="0.0.0.0", port=ALGO_CONTROLLER_PORT)
    server = AlgoServer(config)
    
    await asyncio.gather(
        process_hub_trades(),
        server.serve()
    )

if __name__ == "__main__":
    asyncio.run(main())

