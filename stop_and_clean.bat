@echo off
REM Script to stop service and clean build files

echo Stopping service and cleaning build files...
echo.

REM Check administrator privileges
net session >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo Administrator privileges required!
    echo Run this script as administrator.
    pause
    exit /b 1
)

REM Stop service if running
echo Stopping service...
net stop "MicrophoneVolumeService" >nul 2>&1

REM Kill any remaining processes
taskkill /f /im MicrophoneVolumeService.exe >nul 2>&1

REM Clean build directories
echo Cleaning build directories...
if exist "x64" rmdir /s /q "x64"
if exist "Debug" rmdir /s /q "Debug"
if exist "Release" rmdir /s /q "Release"

REM Clean Visual Studio files
if exist "*.vcxproj.filters" del /q "*.vcxproj.filters"
if exist "*.vcxproj.user" del /q "*.vcxproj.user"

echo Build files cleaned!
echo You can now run build.bat
echo.
pause
