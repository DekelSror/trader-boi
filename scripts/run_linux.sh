#!/bin/bash

# Function to check if Python 3.12 is available
check_python() {
    if ! command -v python3.12 &> /dev/null; then
        echo "Python 3.12 is required but not found."
        echo "Please install Python 3.12 first."
        exit 1
    fi
}

# Function to create and activate virtual environment
setup_venv() {
    if [ ! -d "venv-3.12" ]; then
        echo "Creating virtual environment..."
        python3.12 -m venv venv-3.12
    fi
    source venv-3.12/bin/activate
}

# Function to install requirements
install_requirements() {
    echo "Installing requirements..."
    pip install -r requirements.txt
}

# Function to start a component in a new terminal
start_component() {
    local component=$1
    local command=$2
    
    if command -v gnome-terminal &> /dev/null; then
        gnome-terminal --title="$component" -- bash -c "source venv-3.12/bin/activate && $command; exec bash"
    elif command -v xterm &> /dev/null; then
        xterm -T "$component" -e "bash -c 'source venv-3.12/bin/activate && $command; exec bash'" &
    elif command -v konsole &> /dev/null; then
        konsole --new-tab -p tabtitle="$component" -e "bash -c 'source venv-3.12/bin/activate && $command; exec bash'" &
    else
        echo "No supported terminal emulator found. Please install gnome-terminal, xterm, or konsole."
        exit 1
    fi
}

# Main execution
echo "Setting up trader-boi environment (Linux)..."

# Check Python version
check_python

# Setup virtual environment
setup_venv

# Install requirements
install_requirements

mkdir -p generated_algos
mkdir -p algo_configs

# Start components
echo "Starting components..."
start_component "Mocker" "python mocker.py"
sleep 2  # Wait for mocker to start
start_component "Hub" "python hub.py"
sleep 2  # Wait for hub to start
start_component "Backend" "python backend.py"
sleep 2  # Wait for backend to start

# Open frontend
echo "Opening frontend..."
open_frontend

echo "All components started!"
echo "To stop all components, close the terminal windows or press Ctrl+C in each one." 