#!/bin/bash

# Make sure all scripts are executable
chmod +x run_linux.sh run_mac.sh 2>/dev/null

# Detect OS and run appropriate script
case "$(uname -s)" in
    Linux*)
        echo "Detected Linux OS"
        ./run_linux.sh
        ;;
    Darwin*)
        echo "Detected macOS"
        ./run_mac.sh
        ;;
    CYGWIN*|MINGW*|MSYS*)
        echo "Detected Windows"
        ./run.bat
        ;;
    *)
        echo "Unknown operating system"
        echo "Please run one of these directly:"
        echo "  Linux:   ./run_linux.sh"
        echo "  macOS:   ./run_mac.sh"
        echo "  Windows: run.bat"
        exit 1
        ;;
esac 