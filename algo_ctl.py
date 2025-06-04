from dataclasses import dataclass
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
import websockets
import json
from contextlib import asynccontextmanager
import asyncio

from algo import Algo
from config import TRADE_HUB_WS_URL, ALGO_CONTROLLER_PORT
from market_types import Trade
from simple_algo import Algorithm

@asynccontextmanager
async def lifespan(app: FastAPI):
    # Startup: create the background task
    trade_processor_task = asyncio.create_task(process_hub_trades())
    yield
    # Shutdown: cleanup
    trade_processor_task.cancel()
    try:
        await trade_processor_task
    except asyncio.CancelledError:
        pass

app = FastAPI(lifespan=lifespan)

# Configure CORS
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

running_algos: list[RunningAlgo] = [RunningAlgo(
    algo_class=Algorithm,
    path='/home/dekel/src/trader-boi/simple_algo.py',
    module_name='simple_algo',
    state='stopped',
    actions=[],
    algo_instance=Algorithm()
)]

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
                    print(f'ACTION! {action}')
                    algo.actions.append(action)



if __name__ == "__main__":
    import uvicorn
    process_hub_trades()
    uvicorn.run(app, host="0.0.0.0", port=ALGO_CONTROLLER_PORT)

