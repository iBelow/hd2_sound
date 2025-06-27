@echo off
echo Building Microphone Volume Service...
echo.

REM Check for MSBuild availability
where msbuild >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo MSBuild not found in PATH. Try running from Developer Command Prompt.
    pause
    exit /b 1
)

REM Clean previous build (if exists)
echo Cleaning previous build...
if exist "x64\Release\MicrophoneVolumeService.exe" (
    echo Removing existing executable...
    del /f "x64\Release\MicrophoneVolumeService.exe" >nul 2>&1
)

REM Build Release x64
echo Building Release x64...
msbuild MicrophoneVolumeService.sln /p:Configuration=Release /p:Platform=x64 /verbosity:minimal /p:UseSharedCompilation=false

if %ERRORLEVEL% neq 0 (
    echo.
    echo Build failed!
    echo Try running stop_and_clean.bat as administrator first.
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo Executable file: x64\Release\MicrophoneVolumeService.exe

REM Show version and size information
if exist "x64\Release\MicrophoneVolumeService.exe" (
    echo.
    echo File information:
    "x64\Release\MicrophoneVolumeService.exe" -version
    echo.
    for %%i in ("x64\Release\MicrophoneVolumeService.exe") do echo File size: %%~zi bytes
    echo Icon: Included from assets\icon.ico
)
echo.
pause
