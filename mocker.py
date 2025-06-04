"""
this here mocker simulates the server end of a direct connection to a stock exchange
"""

import socket
import time
import random
import pickle
import datetime

from config import MOCKER_PORT
from market_types import Trade


# Server configuration
HOST = '127.0.0.1'
PORT = 9000

# Symbols to use for mock data
SYMBOLS = ['AAA']
SIDES = ['BUY', 'SELL']
CONDITIONS = ['REGULAR', 'ODD_LOT', 'OUTSIDE_REGULAR_HOURS']

class TimeSimulator:
    def __init__(self, start_time=None, time_step=60):
        """
        Initialize the time simulator
        
        Args:
            start_time: Starting timestamp (default: 24 hours ago)
            time_step: Time to advance in seconds between trades
        """
        # Default to 24 hours ago if not specified
        if start_time is None:
            self.current_time = int(time.time()) - (24 * 60 * 60)
        else:
            self.current_time = start_time
            
        self.time_step = time_step
        
        # Print simulation start time
        start_time_str = datetime.datetime.fromtimestamp(self.current_time).strftime('%Y-%m-%d %H:%M:%S')
        print(f"Time simulation starting from: {start_time_str}")
        print(f"Time step: {time_step} seconds between trades")
    
    def get_time(self):
        """Get the current simulated time"""
        return self.current_time
    
    def advance(self):
        """Advance the simulated time by the time step"""
        self.current_time += self.time_step
        return self.current_time
    
    def get_formatted_time(self):
        """Get the current time formatted as a string"""
        return datetime.datetime.fromtimestamp(self.current_time).strftime('%Y-%m-%d %H:%M:%S')

def generate_mock_trade(time_simulator):
    """Generate a random trade object with simulated time"""
    timestamp = time_simulator.get_time()
    symbol = random.choice(SYMBOLS)
    price = round(random.uniform(100, 1000), 2)
    volume = random.randint(1, 1000)
    side = random.choice(SIDES)
    condition = random.choice(CONDITIONS)
    
    return Trade(
        timestamp=timestamp,
        price=price,
        volume=volume,
        side=side,
        condition=condition,
        symbol=symbol
    )

def start_server():
    """Start the server to send mock trades"""
    # Initialize time simulator
    time_simulator = TimeSimulator(None, 30)
    
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    try:
        server_socket.bind(('127.0.0.1', MOCKER_PORT))
        server_socket.listen(5)
        print(f"Server started on 127.0.0.1:MOCKER_PORT")
        
        while True:
            client_socket, address = server_socket.accept()
            print(f"Connection from {address} established")
            
            try:
                while True:
                    trade = generate_mock_trade(time_simulator)
                    serialized_trade = pickle.dumps(trade)
                    client_socket.sendall(serialized_trade)
                    print(f"Sent trade at {time_simulator.get_formatted_time()}: {trade}")
                    
                    time_simulator.advance()
                    
                    time.sleep(0.5)
            except Exception as e:
                print(f"Error sending data: {e}")
            finally:
                client_socket.close()
                print(f"Connection with {address} closed")
    except KeyboardInterrupt:
        print("Server shutting down...")
    except Exception as e:
        print(f"Server error: {e}")
    finally:
        server_socket.close()
        print("Server closed")


start_server()