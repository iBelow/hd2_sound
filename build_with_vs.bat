@echo off
echo Building Microphone Volume Service with VS Build Tools...
echo.

REM Try to find and initialize Visual Studio environment
set "VS_PATHS=C:\Program Files\Microsoft Visual Studio\2022\Professional;C:\Program Files\Microsoft Visual Studio\2022\Community;C:\Program Files\Microsoft Visual Studio\2022\Enterprise;C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional;C:\Program Files (x86)\Microsoft Visual Studio\2019\Community;C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise"

for %%p in (%VS_PATHS%) do (
    if exist "%%p\Common7\Tools\VsDevCmd.bat" (
        echo Found Visual Studio at: %%p
        call "%%p\Common7\Tools\VsDevCmd.bat" -arch=x64
        goto :build
    )
)

REM Try Build Tools path
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat" (
    echo Found Build Tools
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat" -arch=x64
    goto :build
)

echo Visual Studio or Build Tools not found!
echo Please install Visual Studio 2019/2022 or Build Tools
pause
exit /b 1

:build
echo.
echo Cleaning previous build...
if exist "x64\Release\MicrophoneVolumeService.exe" del /f "x64\Release\MicrophoneVolumeService.exe" >nul 2>&1

echo Building Release x64...
msbuild MicrophoneVolumeService.sln /p:Configuration=Release /p:Platform=x64 /verbosity:minimal /p:UseSharedCompilation=false

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
