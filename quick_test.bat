@echo off
REM Quick test runner for specific test categories

if "%1"=="" (
    echo Usage: quick_test.bat [test_type]
    echo.
    echo Available test types:
    echo   cmdline  - Command line parsing tests
    echo   logging  - Logging functionality tests
    echo   volume   - Volume control tests
    echo   helpers  - Test helper function tests
    echo   all      - Run all tests
    echo.
    echo Examples:
    echo   quick_test.bat cmdline
    echo   quick_test.bat logging
    echo   quick_test.bat all
    pause
    exit /b 1
)

set TEST_EXE=tests\x64\Release\SimpleTests\SimpleTests.exe
if not exist "%TEST_EXE%" (
    set TEST_EXE=tests\x64\Debug\SimpleTests\SimpleTests.exe
)

if not exist "%TEST_EXE%" (
    echo [ERROR] Test executable not found!
    echo Please run run_tests.bat first to build the tests
    echo.
    pause
    exit /b 1
)

REM Note: Our simplified test framework doesn't support filtering like Google Test
REM So we'll just run all tests and indicate which category the user requested

echo ========================================
echo Quick Test Runner
echo ========================================
echo Requested test category: %1
echo.
echo Note: Running all tests (filtering not supported in simplified framework)
echo Look for tests matching your category in the output below:
echo.

if "%1"=="cmdline" (
    echo Focus on: CommandLine_* tests
) else if "%1"=="logging" (
    echo Focus on: Logging_* tests
) else if "%1"=="volume" (
    echo Focus on: Volume_* tests
) else if "%1"=="helpers" (
    echo Focus on: TestHelpers_* tests
) else if "%1"=="all" (
    echo Running: All tests
) else (
    echo [WARNING] Unknown test type: %1
    echo Running all tests anyway...
)

echo ========================================

REM Run all tests
"%TEST_EXE%"

echo.
echo ========================================
echo Quick test execution complete.
echo.
echo To add test filtering, consider upgrading to Google Test framework.
echo See TESTING.md for more information.
echo ========================================
pause
