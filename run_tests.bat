@echo off
REM Script to build and run tests for Microphone Volume Service

echo ========================================
echo Microphone Volume Service Test Runner
echo ========================================
echo.

REM Check administrator privileges for service tests
net session >nul 2>&1
if %ERRORLEVEL% neq 0 (
    echo [WARNING] Not running as administrator
    echo Some service installation/uninstallation tests will be skipped
    echo Run as administrator for full test coverage
    echo.
) else (
    echo [INFO] Running with administrator privileges - all tests available
    echo.
)

REM Check if simple test project exists
if not exist "tests\SimpleTests.vcxproj" (
    echo [ERROR] Test project not found!
    echo Make sure you're in the project root directory
    echo.
    pause
    exit /b 1
)

REM Build the test project
echo [1/2] Building test project (Release x64)...
"C:\Program Files\Microsoft Visual Studio\2022\Community\Msbuild\Current\Bin\amd64\MSBuild.exe" tests\SimpleTests.vcxproj /p:Configuration=Release /p:Platform=x64 /v:minimal

if %ERRORLEVEL% neq 0 (
    echo [ERROR] Test build failed!
    echo.
    echo Building with Debug configuration as fallback...
    "C:\Program Files\Microsoft Visual Studio\2022\Community\Msbuild\Current\Bin\amd64\MSBuild.exe" tests\SimpleTests.vcxproj /p:Configuration=Debug /p:Platform=x64 /v:minimal
    
    if %ERRORLEVEL% neq 0 (
        echo [ERROR] Both Release and Debug builds failed!
        echo Check the build output for errors
        pause
        exit /b 1
    )
    
    set TEST_CONFIG=Debug
    echo [OK] Debug build successful
) else (
    set TEST_CONFIG=Release
    echo [OK] Release build successful
)
echo.

REM Check if test executable was created
set TEST_EXE=tests\x64\%TEST_CONFIG%\SimpleTests\SimpleTests.exe
if not exist "%TEST_EXE%" (
    echo [ERROR] Test executable not found at: %TEST_EXE%
    echo Build may have failed silently
    echo.
    echo Available test files:
    dir tests\x64\%TEST_CONFIG%\SimpleTests\ 2>nul
    pause
    exit /b 1
)

echo [2/2] Test executable found: %TEST_EXE%
echo.

REM Run the tests
echo ========================================
echo Running Tests...
echo ========================================
"%TEST_EXE%"

set TEST_RESULT=%ERRORLEVEL%

echo.
echo ========================================
echo Test Execution Complete
echo ========================================

if %TEST_RESULT% equ 0 (
    echo [SUCCESS] All tests passed!
    echo.
    echo The following functionality has been tested:
    echo - Command line argument parsing
    echo - Logging system (file-based)
    echo - Volume state tracking
    echo - Service utility functions
    echo - File operations and helpers
) else (
    echo [FAILURE] Some tests failed (Exit code: %TEST_RESULT%)
    echo.
    echo Check the output above for details
)

echo.
echo Note: These tests use a simplified test framework
echo For more advanced testing, consider installing Google Test
echo.
pause
exit /b %TEST_RESULT%
