


import json
import websockets
from config import TRADE_HUB_WS_URL
from market_types import Trade
from real_time_accessor import RealTimeAccessor


class WebsocketsAccessor(RealTimeAccessor):
    async def run(self, callback):
        async with websockets.connect(TRADE_HUB_WS_URL) as hub_ws:
            while True:
                trade_data = await hub_ws.recv()
                trade: Trade = Trade(**json.loads(trade_data))
                callback(trade)