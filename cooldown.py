

from time import time


class Cooldown:
    def __init__(self, cooldown_seconds: int) -> None:
        self.cd = cooldown_seconds
        self.last = 0.0
    

    def still(self) -> bool:
        return time() <= self.last + self.cd


    def reset(self):
        self.last = time()
