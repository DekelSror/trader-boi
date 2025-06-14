

from time import time


class Cooldown:
    def __init__(self, cooldown_seconds: int) -> None:
        self.interval = cooldown_seconds
        self.last = 0.0
    

    def still(self) -> bool:
        return time() <= self.last + self.interval


    def reset(self):
        self.last = time()
