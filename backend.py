from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware

import asyncio
import uvicorn

from algo_types import RunningAlgo
from config import ALGO_CONTROLLER_PORT
from engine import create_algo, load_existing_algos
from market_types import Trade
from websockets_accessor import WebsocketsAccessor


app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # In production, replace with specific origin
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


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
async def post_create_algo(body: dict):
    print(f'create algo {body=}')
    try:
        new_algo = create_algo(body)    
        running_algos.append(new_algo)
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


class AlgoServer(uvicorn.Server):
    def __init__(self, config: uvicorn.Config):
        super().__init__(config)
        self.config = config
        self.config.log_level = 'debug'

    async def serve(self):
        self.config.setup_event_loop()
        return await super().serve()

async def main():
    running_algos.extend(load_existing_algos())

    config = uvicorn.Config(app, host="0.0.0.0", port=ALGO_CONTROLLER_PORT)
    server = AlgoServer(config)

    await asyncio.gather(
        WebsocketsAccessor().run(on_trade),
        server.serve()
    )

if __name__ == "__main__":
    asyncio.run(main())

