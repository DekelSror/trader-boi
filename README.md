# how to use
1. open two terminals in the repo root
1. ```make``` in either of them
1. first ```./mocker```
1. then ```./client```
1. client gets the parsed message and aggregates a 5 minute candle chart

# overall arch (plan)
* one client process receives market data messages
    - stores parsed messages in global timeseries db
    - calls agg processes' on_msg handlers - dlsym or IPC ?
* alg manager
    - start / stop algo
    - hot reload / addition of algos
* it manages a bunch of algos. They will 
    - listen to events
    - access timeseries and agg data
    - issue orders
* alg studio
    - create / edit algos
    - compile and add to alg manager