

from market_types import Trade


class Algo:
    def on_trade(self, trade: Trade) -> str | None:
        pass