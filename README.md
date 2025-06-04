# Market Data Processing System

A flexible, extensible system for processing market data, creating aggregations, and running trading algorithms.

## How to Use

### Prerequisites

- Python 3.8+
- Docker (for InfluxDB)
- GCC (for building C algorithms)

### Setup

1. Install dependencies:
   ```
   make install
   ```

2. Start InfluxDB (requires Docker):
   ```
   make setup-db
   ```

3. Build algorithm libraries:
   ```
   make build-algorithms
   ```

### Running the System

1. Start market data simulator in one terminal:
   ```
   make run-mocker
   ```

2. Start the backend processor in another terminal:
   ```
   make run-backend
   ```

3. (Optional) Set up default aggregations and algorithms:
   ```
   make setup-defaults
   ```

## Managing Aggregations

Use the `control.py` tool to manage aggregations:

### List Aggregations
```
python control.py agg-list
```

### Add an Aggregation
```
python control.py agg-add --type ohlc --name ohlc_5min --config interval_seconds=300
```

### Remove an Aggregation
```
python control.py agg-remove --name ohlc_5min
```

## Managing Algorithms

Use the `control.py` tool to manage algorithms:

### List Algorithms
```
python control.py alg-list
```

### Add a Python Algorithm
```
python control.py alg-add --type python --name ma_crossover --path algorithms/simple_ma_crossover.py --config symbol=AAPL short_window=5 long_window=20
```

### Add a C/C++ Algorithm (.so file)
```
python control.py alg-add --type so --name trend_following --path algorithms/trend_following.so --config symbol=MSFT trend_window=10
```

### Restart an Algorithm
```
python control.py alg-restart --name ma_crossover
```

### Remove an Algorithm
```
python control.py alg-remove --name ma_crossover
```

## Architecture

- Refer to ARCHITECTURE.md for a detailed description of the system architecture.

## Extending the System

### Creating New Aggregations

1. Add a new Python module in the `aggregations/` directory
2. Implement the `Aggregator` class with:
   - `__init__(self, publisher, **config)` method
   - `on_trade(self, trade)` method
   - `shutdown(self)` method

### Creating New Algorithms

#### Python Algorithms
1. Add a new Python module in the `algorithms/` directory
2. Implement the `Algorithm` class with:
   - `__init__(self, **config)` method
   - `on_data(self, data)` method
   - `restart(self)` method
   - `shutdown(self)` method

#### C/C++ Algorithms
1. Create a C/C++ source file in the `algorithms/` directory
2. Implement:
   - `int initialize(const char* config_json)` function
   - `const char* process_data(const char* data_json)` function
   - `int shutdown()` function
3. Compile to a shared library (.so file)
4. Add to the system using the control tool