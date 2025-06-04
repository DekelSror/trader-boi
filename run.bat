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
if not exist venv (
    echo Creating virtual environment...
    python3.12 -m venv venv
)

:: Function to start a component in a new terminal
:start_component
start "Activating venv" cmd /c "cd /d %CD% && venv\Scripts\activate.bat && python %~1 && pause"
exit /b

:: Main execution
echo Setting up trader-boi environment (Windows)...

:: Activate venv in current window
call venv\Scripts\activate.bat

:: Install requirements
echo Installing requirements...
pip install -r requirements.txt

:: Start components
echo Starting components...
call :start_component mocker.py
timeout /t 2 > nul
call :start_component hub.py
timeout /t 2 > nul
call :start_component algo_ctl.py
timeout /t 2 > nul

:: Open frontend in default browser
echo Opening frontend...
start "" "frontend\index.html"
start "" "frontend\algo_manager.html"

echo All components started!
echo To stop all components, close the terminal windows or press Ctrl+C in each one.

endlocal 