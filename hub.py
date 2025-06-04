"""
Hub that receives trade data from mocker and broadcasts to WebSocket clients
which are alg_ctl and the frontend
"""
import asyncio
import websockets
import socket
import pickle
import json
from config import MOCKER_PORT, TRADE_HUB_WS_PORT
from market_types import Trade

# Set of connected WebSocket clients
clients = set()

async def register(websocket):
    """Handle new WebSocket client connections"""
    clients.add(websocket)
    try:
        await websocket.wait_closed()
    finally:
        clients.remove(websocket)
        print(f"Client disconnected. {len(clients)} clients remaining")

async def receive_and_broadcast():
    """Receive trades from mocker and broadcast to WebSocket clients"""
    # Connect to mocker
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect(('127.0.0.1', MOCKER_PORT))  # Connect to mocker's port
    sock.setblocking(False)
    print("Connected to mocker")
    
    loop = asyncio.get_event_loop()
    
    while True:
        try:
            # Non-blocking receive from mocker
            raw_trade = await loop.sock_recv(sock, 4096)
            if not raw_trade:
                print("Mocker connection closed")
                break
                
            trade: Trade = pickle.loads(raw_trade)
            trade_dict = {
                'timestamp': trade.timestamp,
                'price': trade.price,
                'volume': trade.volume,
                'side': trade.side,
                'condition': trade.condition,
                'symbol': trade.symbol
            }
            
            if clients:
                message = json.dumps(trade_dict)
                websockets.broadcast(clients, message)
                print(f"Broadcasted trade to {len(clients)} clients: {trade_dict}")
            
        except Exception as e:
            print(f"Error receiving/broadcasting trade: {e}")
            break

async def main():
    asyncio.create_task(receive_and_broadcast())
    
    async with websockets.serve(register, "localhost", TRADE_HUB_WS_PORT):
        print(f"Hub running - WebSocket server on ws://localhost:{TRADE_HUB_WS_PORT}")
        await asyncio.Future()  # run forever

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nHub shutting down...")

