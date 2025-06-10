// Available trade properties for conditions
const TRADE_PROPS = ['price', 'volume', 'side', 'symbol'];
const OPERATORS = ['>', '<', '>=', '<=', '==', '!='];
const ACTIONS = ['BUY', 'SELL'];

function createConditionElement() {
    const div = document.createElement('div');
    div.className = 'condition';
    
    const propSelect = document.createElement('select');
    TRADE_PROPS.forEach(prop => {
        const option = document.createElement('option');
        option.value = prop;
        option.textContent = prop;
        propSelect.appendChild(option);
    });

    const opSelect = document.createElement('select');
    OPERATORS.forEach(op => {
        const option = document.createElement('option');
        option.value = op;
        option.textContent = op;
        opSelect.appendChild(option);
    });

    const valueInput = document.createElement('input');
    valueInput.type = 'number';
    valueInput.value = '0';

    const removeBtn = document.createElement('button');
    removeBtn.textContent = 'x';
    removeBtn.className = 'remove-btn';
    removeBtn.onclick = () => {
        div.remove();
        updatePreview();
    };

    div.append(propSelect, opSelect, valueInput, removeBtn);
    return div;
}

function addRule() {
    const rulesDiv = document.getElementById('rules');
    const ruleDiv = document.createElement('div');
    ruleDiv.className = 'param-row';

    // Action selection
    const actionSelect = document.createElement('select');
    ACTIONS.forEach(action => {
        const option = document.createElement('option');
        option.value = action;
        option.textContent = action;
        actionSelect.appendChild(option);
    });

    // Conditions container
    const conditionsDiv = document.createElement('div');
    conditionsDiv.className = 'rule-conditions';
    
    // Add condition button
    const addCondBtn = document.createElement('button');
    addCondBtn.textContent = 'Add Condition';
    addCondBtn.onclick = () => {
        conditionsDiv.appendChild(createConditionElement());
        updatePreview();
    };

    // Remove rule button
    const removeBtn = document.createElement('button');
    removeBtn.textContent = 'x';
    removeBtn.className = 'remove-btn';
    removeBtn.onclick = () => {
        ruleDiv.remove();
        updatePreview();
    };

    ruleDiv.append(
        document.createTextNode('If conditions met, then: '),
        actionSelect,
        removeBtn,
        document.createElement('br'),
        conditionsDiv,
        addCondBtn
    );

    rulesDiv.appendChild(ruleDiv);
    
    // Add first condition automatically
    conditionsDiv.appendChild(createConditionElement());
    updatePreview();
}

function gatherConfig() {
    const config = {
        title: document.getElementById('algoTitle').value,
        algo_params: {
            cooldown: parseInt(document.getElementById('cooldown').value)
        },
        rules: []
    };

    const rulesDiv = document.getElementById('rules');
    rulesDiv.querySelectorAll('.param-row').forEach(ruleDiv => {
        const action = ruleDiv.querySelector('select').value;
        const conditions = [];
        
        ruleDiv.querySelectorAll('.condition').forEach(condDiv => {
            const selects = condDiv.querySelectorAll('select');
            const input = condDiv.querySelector('input');
            conditions.push([`trade.${selects[0].value}`, selects[1].value, parseFloat(input.value)]);
        });

        config.rules.push({
            conditions,
            action
        });
    });

    return config;
}

function updatePreview() {
    const config = gatherConfig();
    document.getElementById('preview').textContent = JSON.stringify(config, null, 2);
}

async function submitAlgo() {
    const config = gatherConfig();
    try {
        const response = await fetch('http://localhost:13000/api/algos', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: JSON.stringify(config)
        });
        
        if (!response.ok) {
            throw new Error('Failed to build algorithm');
        }
        
        alert('Algorithm built successfully!');
    } catch (e) {
        alert('Error: ' + e.message);
    }
}

// Add event listeners to update preview
document.body.addEventListener('change', updatePreview);
document.body.addEventListener('input', updatePreview);

// Add initial rule when page loads
window.addEventListener('load', () => {
    addRule();
    updatePreview();
});