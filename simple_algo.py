from algo import Algo
from cooldown import Cooldown

class Algorithm(Algo):
    def __init__(self) -> None:
        # initialize params
        self.cooldown = Cooldown(120)
        # get aggregations

    def on_trade(self, trade):
        # prerequisites
        if self.cooldown.still():
            return None

        # rules
        action = None
        if trade.price > 700:
            action = 'SELL'
        elif trade.price < 400:
            action = 'BUY'

        if action:
            self.cooldown.reset()
        return action
    
