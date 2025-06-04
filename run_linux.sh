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
    if [ ! -d "venv" ]; then
        echo "Creating virtual environment..."
        python3.12 -m venv venv
    fi
    source venv/bin/activate
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
        gnome-terminal --title="$component" -- bash -c "source venv/bin/activate && $command; exec bash"
    elif command -v xterm &> /dev/null; then
        xterm -T "$component" -e "bash -c 'source venv/bin/activate && $command; exec bash'" &
    elif command -v konsole &> /dev/null; then
        konsole --new-tab -p tabtitle="$component" -e "bash -c 'source venv/bin/activate && $command; exec bash'" &
    else
        echo "No supported terminal emulator found. Please install gnome-terminal, xterm, or konsole."
        exit 1
    fi
}

# Function to open HTML files in browser
open_frontend() {
    if command -v xdg-open &> /dev/null; then
        xdg-open "frontend/index.html"
        xdg-open "frontend/algo_manager.html"
    elif command -v firefox &> /dev/null; then
        firefox "frontend/index.html" "frontend/algo_manager.html" &
    elif command -v google-chrome &> /dev/null; then
        google-chrome "frontend/index.html" "frontend/algo_manager.html" &
    elif command -v chromium &> /dev/null; then
        chromium "frontend/index.html" "frontend/algo_manager.html" &
    else
        echo "No supported browser found. Please open the following files manually:"
        echo "frontend/index.html"
        echo "frontend/algo_manager.html"
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