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

REM Build Release x64
echo Building Release x64...
msbuild MicrophoneVolumeService.sln /p:Configuration=Release /p:Platform=x64 /verbosity:minimal

if %ERRORLEVEL% neq 0 (
    echo.
    echo Build failed!
    pause
    exit /b 1
)

echo.
echo Build completed successfully!
echo Executable file: x64\Release\MicrophoneVolumeService.exe
echo.
pause
