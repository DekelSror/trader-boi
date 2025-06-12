def generate_algo_code(config: dict) -> str:
    """Generate algorithm code from a configuration dictionary"""
    # Infer class name from config['title']
    algo_class = ''.join(config['title'].split(' '))
    # Start with imports
    code = [
        "from algo import Algo",
        "from cooldown import Cooldown\n",
        f"class {algo_class}(Algo):",
        "    def __init__(self) -> None:",
        "        # initialize params",
    ]
    
    # Handle initialization params
    params = config.get('algo_params', {})
    if 'cooldown' in params:
        code.append(f"        self.cooldown = Cooldown({params['cooldown']})")
    code.append("        # get aggregations\n")
    
    # Start on_trade method
    code.extend([
        "    def on_trade(self, trade):",
        "        # prerequisites"
    ])
    
    # Handle prerequisites (like cooldown)
    if 'cooldown' in params:
        code.extend([
            "        if self.cooldown.still():",
            "            return None\n"
        ])
    
    # Handle rules section
    code.extend([
        "        # rules",
        "        action = None"
    ])
    
    # Generate rule conditions
    rules = config.get('rules', [])
    for i, rule in enumerate(rules):
        conditions = rule['conditions']
        action = rule['action']
        
        # Build condition expression
        if len(conditions) == 1:
            attr, op, value = conditions[0]
            code.append(f"        {'elif' if i > 0 else 'if'} trade.{attr.split('.')[-1]} {op} {value}:")
        else:
            # Handle multiple conditions (AND)
            conditions_str = " and ".join(f"trade.{c[0].split('.')[-1]} {c[1]} {c[2]}" for c in conditions)
            code.append(f"        {'elif' if i > 0 else 'if'} {conditions_str}:")
        code.append(f"            action = '{action}'")
    
    # Handle post-rule actions
    code.extend([
        "\n        if action:",
        "            self.cooldown.reset()" if 'cooldown' in params else "            pass",
        "        return action",
        "    \n"
    ])
    
    return "\n".join(code)

def build_algo_file(config: dict, output_path: str) -> None:
    """Build an algorithm file from a configuration dictionary"""
    code = generate_algo_code(config)
    with open(output_path, 'w') as f:
        f.write(code)