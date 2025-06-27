@echo off
REM Script to test service without installation

echo Testing Microphone Volume Service...
echo.

REM Check for executable file
if not exist "x64\Release\MicrophoneVolumeService.exe" (
    echo Executable file not found!
    echo Build the project first (build.bat).
    pause
    exit /b 1
)

echo Running in test mode...
echo Interval: 2 seconds (default)
echo Microphones: all active
echo.
echo Press Ctrl+C to stop
echo.

x64\Release\MicrophoneVolumeService.exe -test

echo.
pause
