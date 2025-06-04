from typing import Callable
from market_types import OHLC, Trade


class OHLCAggregator:
    def __init__(self, symbol: str, interval_seconds, post_candle: Callable[[OHLC], None]):
        self.interval_seconds = interval_seconds
        self.symbol = symbol
        self.candle: OHLC | None = None
        self.post_candle = post_candle
        
    def on_trade(self, trade: Trade):
        if self.candle is None:
            self.candle = OHLC(
                open=trade.price,
                high=trade.price,
                low=trade.price,
                close=trade.price,
                timestamp=trade.timestamp,
                symbol=trade.symbol,
            )

            return
        
        if trade.price > self.candle.high:
            self.candle.high = trade.price
        if trade.price < self.candle.low:
            self.candle.low = trade.price
        
        if trade.timestamp - self.candle.timestamp >= self.interval_seconds:
            print(f'posting candle {self.candle}')
            self.candle.close = trade.price
            self.post_candle(self.candle)
            self.candle = None
            

