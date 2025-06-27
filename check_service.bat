@echo off
REM Script to check service status and configuration

echo Microphone Volume Service Status Check
echo ======================================
echo.

REM Check if service exists
sc query "MicrophoneVolumeService" >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo [ERROR] Service is not installed!
    echo Run install_service.bat to install the service.
    echo.
    pause
    exit /b 1
)

echo [INFO] Service is installed
echo.

REM Show service status
echo Service Status:
echo ---------------
sc query "MicrophoneVolumeService"
echo.

REM Show service configuration
echo Service Configuration:
echo ----------------------
sc qc "MicrophoneVolumeService"
echo.

REM Check if service is running
sc query "MicrophoneVolumeService" | find "RUNNING" >nul
if %ERRORLEVEL% equ 0 (
    echo [OK] Service is currently RUNNING
) else (
    echo [WARNING] Service is NOT running
    echo.
    set /p "choice=Do you want to start the service now? (y/n): "
    if /i "%choice%"=="y" (
        echo Starting service...
        net start "MicrophoneVolumeService"
        if %ERRORLEVEL% equ 0 (
            echo [OK] Service started successfully
        ) else (
            echo [ERROR] Failed to start service
        )
    )
)

echo.

REM Check auto-start configuration
sc qc "MicrophoneVolumeService" | find "AUTO_START" >nul
if %ERRORLEVEL% equ 0 (
    echo [OK] Service is configured for AUTO-START
) else (
    echo [WARNING] Service is NOT configured for auto-start
    echo.
    set /p "choice=Do you want to enable auto-start? (y/n): "
    if /i "%choice%"=="y" (
        echo Configuring auto-start...
        sc config "MicrophoneVolumeService" start= auto
        if %ERRORLEVEL% equ 0 (
            echo [OK] Auto-start enabled
        ) else (
            echo [ERROR] Failed to enable auto-start
        )
    )
)

echo.
echo Log file location: C:\Windows\Temp\MicrophoneVolumeService.log
echo Service management: services.msc
echo.
pause
