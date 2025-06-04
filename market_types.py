
from dataclasses import dataclass


@dataclass
class OHLC:
    open: float
    high: float
    low: float
    close: float
    symbol: str
    timestamp: int

@dataclass
class Trade:
    timestamp: int
    price: float
    volume: int
    side: str
    condition: str
    symbol: str