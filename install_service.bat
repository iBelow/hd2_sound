@echo off
REM Script to install service with administrator privileges

echo Installing Microphone Volume Service...
echo.

REM Check administrator privileges
net session >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Administrator privileges required to install service!
    echo Run this script as administrator.
    pause
    exit /b 1
)

REM Check for executable file
if not exist "x64\Release\MicrophoneVolumeService.exe" (
    echo Executable file not found!
    echo Build the project first (build.bat).
    pause
    exit /b 1
)

REM Stop service if running
echo Stopping service (if running)...
net stop "MicrophoneVolumeService" >nul 2>&1

REM Remove existing service if present
echo Removing existing service (if present)...
x64\Release\MicrophoneVolumeService.exe -uninstall >nul 2>&1

REM Install service with default parameters
echo Installing service...
x64\Release\MicrophoneVolumeService.exe -install

if %ERRORLEVEL% neq 0 (
    echo Service installation failed!
    pause
    exit /b 1
)

REM Start service
echo Starting service...
net start "MicrophoneVolumeService"

if %ERRORLEVEL% neq 0 (
    echo Service installed but failed to start.
    echo Try starting manually through services.msc
) else (
    echo Service successfully installed and started!
)

echo.
echo Service log: C:\Windows\Temp\MicrophoneVolumeService.log
echo Service management: services.msc
echo.
pause
