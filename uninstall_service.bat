@echo off
REM Script to uninstall service with administrator privileges

echo Uninstalling Microphone Volume Service...
echo.

REM Check administrator privileges
net session >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Administrator privileges required to uninstall service!
    echo Run this script as administrator.
    pause
    exit /b 1
)

REM Check for executable file
if not exist "x64\Release\MicrophoneVolumeService.exe" (
    echo Executable file not found!
    echo Service may already be removed or file moved.
    pause
    exit /b 1
)

REM Stop service
echo Stopping service...
net stop "MicrophoneVolumeService"

REM Uninstall service
echo Uninstalling service...
x64\Release\MicrophoneVolumeService.exe -uninstall

if %ERRORLEVEL% neq 0 (
    echo Service uninstallation failed!
    pause
    exit /b 1
)

echo Service successfully uninstalled!
echo.
pause
