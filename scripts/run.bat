@echo off
setlocal enabledelayedexpansion

:: Function to check Python 3.12
where python3.12 > nul 2>&1
if errorlevel 1 (
    echo Python 3.12 is required but not found.
    echo Please install Python 3.12 from python.org
    exit /b 1
)

:: Create and activate virtual environment
if not exist venv-3.12 (
    echo Creating virtual environment...
    python3.12 -m venv venv-3.12
)

:: Function to start a component in a new terminal
:start_component
start "Activating venv" cmd /c "cd /d %CD% && venv-3.12\Scripts\activate.bat && python %~1 && pause"
exit /b

:: Main execution
echo Setting up trader-boi environment (Windows)...

:: Activate venv in current window
call venv-3.12\Scripts\activate.bat

:: Verify required directories exist
echo Verifying required directories...
if not exist "algo_configs" (
    echo Creating algo_configs directory...
    mkdir "algo_configs"
)
if not exist "generated_algos" (
    echo Creating generated_algos directory...
    mkdir "generated_algos"
)

:: Install requirements
echo Installing requirements...
pip install -r requirements.txt

:: Start components
echo Starting components...
call :start_component mocker.py
timeout /t 2 > nul
call :start_component hub.py
timeout /t 2 > nul
call :start_component backend.py
timeout /t 2 > nul

:: Open frontend in default browser
echo Opening frontend...
start "" "frontend\algo_builder.html"
start "" "frontend\algo_manager.html"

echo All components started!
echo To stop all components, close the terminal windows or press Ctrl+C in each one.

endlocal 