

from market_types import Trade


class MovingAverage:
    def __init__(self, window_size: int) -> None:
        self.trades: list[Trade] = []
        self.window_size = window_size
        self.value: float | None = None

    def on_trade(self, trade: Trade):
        if len(self.trades < self.window_size):
            self.trades.append(trade)
            return
        
        self.trades = self.trades[1::]
        self.trades.append(trade)

        self.value = sum(t.price for t in self.trades) / self.window_size
