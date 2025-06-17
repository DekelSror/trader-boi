from dataclasses import dataclass, field
import json
import importlib.util
import os
import glob

from algo import Algo
from algo_types import RunningAlgo
from algo_types import AlgoSchema, AlgoParam, AlgoRule, AlgoCondition

# "compiler"

def parse_schema(user_input: dict) -> AlgoSchema:
    params = []
    for p in user_input['params']:
        param_type = p['param_type']
        init_args = p['init_args']
        params.append(AlgoParam(param_type=param_type, init_args=init_args))
    
    rules = []
    for rule in user_input['rules']:
        conditions = []
        for condition in rule['conditions']:
            left = condition['left']
            op = condition['op']
            right = condition['right']
            conditions.append(AlgoCondition(left=left, op=op, right=right))
        rules.append(AlgoRule(conditions=conditions, action=rule['action']))
    
    return AlgoSchema(
        title=user_input['title'],
        params=params,
        rules=rules
    )

@dataclass
class CodeStatement:
    st: str
    children: list = field(default_factory=list)


def write_code_statement(cs: CodeStatement, depth: int):
    tabbed_st = '\t' * depth + cs.st
    if not cs.children:
        return tabbed_st

    res: list[str] = [tabbed_st]
    for sub_statement in cs.children:
        res.append(write_code_statement(sub_statement, depth + 1))
    
    return '\n'.join(res)


def write_algo_param(param: AlgoParam, param_class_name: str, i: int):
    parsed_args = ''
    for arg in param.init_args:
        if type(arg) is str:
            parsed_args += "'" + arg + "'"
        else:
            parsed_args += str(arg)
        
        parsed_args += ','
    
    parsed_args = parsed_args[:-1:]

    return CodeStatement(st=f'self.param_{i} = {param_class_name}({parsed_args})')


def write_algo_condition(condition: AlgoCondition, i: int):
    left = condition.left
    if left.startswith('param_'):
        left = f'self.{left}'

    op = condition.op
    right = condition.right

    return CodeStatement(
        st=f'rule_{i} += ({left} {op} {right})',
        children=[]
    )


def write_algo(schema: AlgoSchema):
    algo_imports: list[str] = []

    algo_imports.append('from algo import Algo')
    algo_imports.append('from market_types import Trade')
    algo_init = CodeStatement(st=f'def __init__(self):')

    for i, algo_param in enumerate(schema.params):
        param_class_name = ''.join(t.capitalize() for t in algo_param.param_type.split('_'))
        algo_imports.append(f'from params.{algo_param.param_type} import {param_class_name}')        
        algo_init.children.append(write_algo_param(algo_param, param_class_name, i))
    
    on_trade = CodeStatement(st=f'def on_trade(self, trade: Trade):')
    for i, rule in enumerate(schema.rules):
        block: list[CodeStatement] = []
        block.append(CodeStatement(st=f'rule_{i} = 0'))

        for condition in rule.conditions:
            block.append(write_algo_condition(condition, i))

        block.append(
            CodeStatement(
                st=f'if rule_{i} == {len(rule.conditions)}:',
                children=[CodeStatement(st=f"'{rule.action}'")]
            )
        )
        on_trade.children.extend(block)

    res = algo_imports
    res.append('\n')

    the_class = CodeStatement(
        st=f'class {''.join(t.capitalize() for t in schema.title.split(' '))}(Algo):',
        children=[on_trade]
    )

    if algo_init.children:
        the_class.children.append(algo_init)

    res.append(write_code_statement(the_class, 0))

    return '\n'.join(res)

# module type stuff

def create_algo(raw: dict):
    algo_title: str = raw['title']
    module_name = '_'.join(algo_title.lower().split(' '))
    code_path = f'generated_algos/{module_name}.py'
    config_path = f'algo_configs/{module_name}.json'

    create_algo_code(raw, module_name, code_path)
    res = load_algo_from_file(module_name, code_path)
    create_algo_config(config_path, res, raw)

    return res


def create_algo_code(raw: dict, module_name: str, code_path: str):
    schema = parse_schema(raw)
    algo_code = write_algo(schema)

    with open(code_path, 'w') as code_file:
        code_file.write(algo_code)


def create_algo_config(config_path: str, ra: RunningAlgo, raw: dict):
    with open(config_path, 'w') as config_file:
        json.dump({
            'algo_title': ra.algo_class.__name__,
            'module_name': ra.module_name,
            'path': ra.path,
            'state': ra.state,
            'schema': raw,
        }, config_file, indent=4)


def load_algo_from_file(module_name: str, file_path: str) -> RunningAlgo:
    """Load an algorithm from a file, auto-discover the Algo subclass, and create a RunningAlgo instance."""
    spec = importlib.util.spec_from_file_location(module_name, file_path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)

    algo_classes = [
        cls for name, cls in vars(module).items()
        if isinstance(cls, type) and issubclass(cls, Algo) and cls is not Algo
    ]
    if len(algo_classes) == 0:
        raise RuntimeError(f"No Algo subclass found in {file_path}")
    if len(algo_classes) > 1:
        raise RuntimeError(f"Multiple Algo subclasses found in {file_path}: {[cls.__name__ for cls in algo_classes]}")
    algo_class = algo_classes[0]
    algo_instance = algo_class()

    # Create and return RunningAlgo instance
    return RunningAlgo(
        algo_class=algo_class,
        path=file_path,
        module_name=module_name,
        algo_instance=algo_instance,
        state='running',
        actions=[]
    )

def load_existing_algos():
    res: list[RunningAlgo] = []
    for file_path in glob.glob('generated_algos/*.py'):
        module_name = os.path.splitext(os.path.basename(file_path))[0]
        try:
            running_algo = load_algo_from_file(module_name, file_path)
            res.append(running_algo)
            print(f'Loaded algorithm: {module_name}')
        except Exception as e:
            print(f'Error loading algorithm {file_path}: {str(e)}')

    return res



# raw: dict = {
#     'title': 'My Awesome Algo',
#     'rules': [
#         {'conditions': [{'left': 'trade.price', 'op': '>', 'right': '500'}], 'action': 'BUY'},
#         {'conditions': [{'left': 'trade.price', 'op': '>', 'right': '500'}], 'action': 'SELL'}],
#     'params': []
# }

# res = create_algo(raw)
# debug = 1

# with open('simple_algo.json', 'r') as source:
#     raw: dict = json.load(source)

#     res = create_algo(raw)
#     debug = 1

