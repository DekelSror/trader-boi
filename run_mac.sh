#!/bin/bash

# Function to check if Python 3.12 is available
check_python() {
    if ! command -v python3.12 &> /dev/null; then
        echo "Python 3.12 is required but not found."
        echo "Please install Python 3.12 first (brew install python@3.12)"
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
    
    # Using osascript to open new Terminal windows
    osascript -e "tell application \"Terminal\"
        do script \"cd $(pwd) && source venv-3.12/bin/activate && $command\"
        set custom title of front window to \"$component\"
    end tell"
}

# Function to open HTML files in browser
open_frontend() {
    open "frontend/index.html"
    open "frontend/algo_manager.html"
}

# Main execution
echo "Setting up trader-boi environment (macOS)..."

# Check Python version
check_python

# Setup virtual environment
setup_venv

# Install requirements
install_requirements

# Start components
echo "Starting components..."
start_component "Mocker" "python mocker.py"
sleep 2  # Wait for mocker to start
start_component "Hub" "python hub.py"
sleep 2  # Wait for hub to start
start_component "Algo Controller" "python algo_ctl.py"
sleep 2  # Wait for algo controller to start

# Open frontend
echo "Opening frontend..."
open_frontend

echo "All components started!"
echo "To stop all components, close the terminal windows or press Ctrl+C in each one." 