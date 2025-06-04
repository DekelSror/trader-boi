"""
Simple Moving Average Crossover Algorithm

A basic trading algorithm that generates buy/sell signals based on moving average crossovers.
"""

import time
from collections import deque
from datetime import datetime

from algo import Algo
from market_types import Trade

class SimpleMACrossoverAlgo(Algo):
    """
    Simple Moving Average Crossover Algorithm
    """
    
    def __init__(self):
        self.symbol = "AAA"
        self.short_window = 5
        self.long_window = 20
        self.interval_seconds = 300
        
        # Initialize price queues
        self.short_prices = deque(maxlen=self.short_window)
        self.long_prices = deque(maxlen=self.long_window)
        
        # Initialize state
        self.last_position = None  # None, "BUY", or "SELL"
        self.last_signal_time = 0
        self.cooldown_seconds = 60  # Minimum time between signals
        
        print(f"MA Crossover initialized for {self.symbol} with {self.short_window}/{self.long_window} windows")
    
    def on_trade(self, trade: Trade):
        """Process a data update (OHLC candle)"""
        # Check if this is the symbol and interval we're interested in
        if trade.symbol != self.symbol:
            return None
        
        # Add closing price to our queues
        close_price = trade.price
        self.short_prices.append(close_price)
        self.long_prices.append(close_price)
        
        # Wait until we have enough data
        if len(self.short_prices) < self.short_window or len(self.long_prices) < self.long_window:
            return None
        
        # Calculate moving averages
        short_ma = sum(self.short_prices) / self.short_window
        long_ma = sum(self.long_prices) / self.long_window
        
        # Generate signal if appropriate
        current_time = time.time()
        if current_time - self.last_signal_time < self.cooldown_seconds:
            return None
        
        signal = None
        
        # Check for crossover
        if short_ma > long_ma and (self.last_position is None or self.last_position == "SELL"):
            signal = "BUY"
            self.last_position = "BUY"
            self.last_signal_time = current_time
        elif short_ma < long_ma and (self.last_position is None or self.last_position == "BUY"):
            signal = "SELL"
            self.last_position = "SELL"
            self.last_signal_time = current_time
        
        if signal:
            time_str = datetime.fromtimestamp(trade.timestamp).strftime('%Y-%m-%d %H:%M:%S')
            print(f"MA Crossover {signal} signal for {self.symbol} at {time_str}")
            print(f"Short MA: {short_ma:.2f}, Long MA: {long_ma:.2f}")
            return {
                "action": signal,
                "symbol": self.symbol,
                "price": close_price,
                "timestamp": trade.timestamp,
                "reason": f"{self.short_window}/{self.long_window} MA Crossover"
            }
        
        return None
    
