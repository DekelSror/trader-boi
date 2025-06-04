from algo import Algo

class Algorithm(Algo):
    def __init__(self) -> None:
        super().__init__()
        self.last_action = 0
        self.action_interval = 300

    def on_trade(self, trade):
        if trade.timestamp < self.action_interval + self.last_action:
            return None

        print(f'{self.last_action=} {trade.timestamp=}')
        if trade.price > 700:
            self.last_action = trade.timestamp
            return 'BUY'
        elif trade.price < 400:
            self.last_action = trade.timestamp
            return 'SELL'
        
        return None
    
