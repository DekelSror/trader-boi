#!/bin/bash

# Make sure all scripts are executable
chmod +x run_linux.sh run_mac.sh 2>/dev/null

# Detect OS and run appropriate script
case "$(uname -s)" in
    Linux*)
        echo "Detected Linux OS"
        ./scripts/run_linux.sh
        ;;
    Darwin*)
        echo "Detected macOS"
        ./scripts/run_mac.sh
        ;;
    CYGWIN*|MINGW*|MSYS*)
        echo "Detected Windows"
        ./scripts/run.bat
        ;;
    *)
        echo "Unknown operating system"
        echo "Please run one of these directly:"
        echo "  Linux:   ./scripts/run_linux.sh"
        echo "  macOS:   ./scripts/run_mac.sh"
        echo "  Windows: scripts\run.bat"
        exit 1
        ;;
esac 