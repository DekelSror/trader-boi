


from abc import ABC

from market_types import Trade


class Algo(ABC):
    def on_trade(self, trade: Trade) -> None:
        pass