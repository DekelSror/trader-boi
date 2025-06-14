from dataclasses import dataclass

from algo import Algo


@dataclass
class AlgoCondition:
    left: str
    op: str
    right: str | float

type AlgoAction = str

@dataclass
class AlgoRule:
    conditions: list[AlgoCondition]
    action: AlgoAction

@dataclass
class AlgoParam:
    param_type: str
    init_args: list[str | int | float | bool | None]

@dataclass
class AlgoSchema:
    title: str
    params: list[AlgoParam]
    rules: list[AlgoRule]

@dataclass
class RunningAlgo:
    algo_class: type[Algo]
    path: str
    module_name: str
    algo_instance: Algo
    state: str  # running, pending, error, stopped
    actions: list[str]
