

import json

from engine import create_algo


with open('simple_algo.json', 'r') as source:
    raw: dict = json.load(source)

    res = create_algo(raw)


# test_schema = AlgoSchema(
#     'test algo',
#     [
#         AlgoParam('cooldown', [3]),
#         AlgoParam('moving_average', ['AAA', 10])
#     ],
#     [
#         AlgoRule(
#             [
#                 AlgoCondition('trade.price', '>', 700.0),
#                 AlgoCondition('param_0', '==', True),
#                 AlgoCondition('param_1', '>', 550.0),
#             ],
#             'BUY'
#         ),
#         AlgoRule(
#             [
#                 AlgoCondition('trade.price', '<', 400.0)
#             ],
#             'SELL'
#         )
#     ]
# )