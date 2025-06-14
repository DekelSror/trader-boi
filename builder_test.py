from algo import Algo
from market_types import Trade
from params.cooldown import Cooldown
from params.moving_average import MovingAverage


class TestAlgo(Algo):
	def __init__(self):
		self.param_0 = Cooldown(3)
		self.param_1 = MovingAverage('AAA',10)

	def on_trade(self, trade: Trade):
		rule_0 = 0
		rule_0 += (trade.price > 700.0)
		rule_0 += (self.param_0 == True)
		rule_0 += (self.param_1 > 550.0)

		if rule_0 == 3:
			'BUY'

		rule_1 = 0
		rule_1 += (trade.price < 400.0)
		if rule_1 == 1:
			'SELL'