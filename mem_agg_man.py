

from typing import Any


aggs: dict[str, list[Any]] = {}



def add_to_agg(name: str, data):
    if not aggs.get(name):
        aggs[name] = []

    aggs[name].append(data)