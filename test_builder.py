import json
from algo_builder import build_algo_file

def main():
    # Read the JSON config
    with open('simple_algo.json', 'r') as f:
        config = json.load(f)
    
    # Generate the Python file
    build_algo_file(config, 'generated_simple_algo.py')
    
    # Compare with original
    with open('simple_algo.py', 'r') as f:
        original = f.read()
    with open('generated_simple_algo.py', 'r') as f:
        generated = f.read()
    
    print("Files generated! Comparison:")
    print("\nOriginal simple_algo.py:")
    print(original)
    print("\nGenerated simple_algo.py:")
    print(generated)

if __name__ == "__main__":
    main() 