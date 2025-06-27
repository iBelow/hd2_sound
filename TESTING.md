# Testing Guide

This document describes the testing infrastructure for the Microphone Volume Service project.

## Overview

The project includes a comprehensive test suite that validates all major functionality including command line parsing, logging, volume control, and service utilities.

## Test Framework

### Simplified Test Framework

The project uses a custom lightweight testing framework that provides:

- **No external dependencies**: Works without Google Test or other frameworks
- **Simple assertions**: Easy-to-use macros like `EXPECT_TRUE`, `EXPECT_EQ`
- **Automatic discovery**: Tests are automatically registered and executed
- **Clear reporting**: Detailed pass/fail results with error messages

### Available Assertions

```cpp
EXPECT_TRUE(condition)          // Expects condition to be true
EXPECT_FALSE(condition)         // Expects condition to be false
EXPECT_EQ(expected, actual)     // Expects values to be equal
EXPECT_NE(expected, actual)     // Expects values to be different
EXPECT_LT(val1, val2)          // Expects val1 < val2
EXPECT_LE(val1, val2)          // Expects val1 <= val2
EXPECT_GT(val1, val2)          // Expects val1 > val2
EXPECT_GE(val1, val2)          // Expects val1 >= val2
EXPECT_FLOAT_EQ(expected, actual) // Expects float equality (with tolerance)
EXPECT_NO_THROW(statement)     // Expects no exception to be thrown
EXPECT_FAILED(hr)              // Expects HRESULT to indicate failure
```

## Test Categories

### 1. Command Line Parsing Tests

**File**: `tests/SimpleTests.cpp` (CommandLine_* functions)

**Purpose**: Validate that command line arguments are parsed correctly

**Tests**:
- `CommandLine_ParseInterval_ValidValue`: Tests valid interval parsing
- `CommandLine_ParseInterval_ZeroValue_UsesDefault`: Tests invalid interval handling
- `CommandLine_ParseMicrophoneFilter`: Tests microphone filter parsing
- `CommandLine_ParseEventLogFlag`: Tests event log flag parsing

### 2. Logging System Tests

**File**: `tests/SimpleTests.cpp` (Logging_* functions)

**Purpose**: Validate the logging system functionality

**Tests**:
- `Logging_WriteLog_CreatesFile`: Tests log file creation
- `Logging_WriteLog_ContainsMessage`: Tests message content in logs
- `Logging_WriteErrorLog_ContainsErrorPrefix`: Tests error log formatting

### 3. Volume Control Tests

**File**: `tests/SimpleTests.cpp` (Volume_* functions)

**Purpose**: Validate volume state tracking and change detection

**Tests**:
- `Volume_LastVolumeState_Tracking`: Tests volume state storage
- `Volume_VolumeChangeDetection`: Tests volume change detection logic

### 4. Helper Function Tests

**File**: `tests/SimpleTests.cpp` (TestHelpers_* functions)

**Purpose**: Validate utility and helper functions

**Tests**:
- `TestHelpers_FileOperations`: Tests file creation, reading, and deletion

## Running Tests

### Quick Start

```cmd
# Build and run all tests
run_tests.bat
```

### Manual Execution

```cmd
# Build tests manually
"C:\Program Files\Microsoft Visual Studio\2022\Community\Msbuild\Current\Bin\amd64\MSBuild.exe" tests\SimpleTests.vcxproj /p:Configuration=Release /p:Platform=x64

# Run tests manually
tests\x64\Release\SimpleTests\SimpleTests.exe
```

### Expected Output

```
========================================
Microphone Volume Service Tests
========================================
Using simplified test framework
========================================
Running: CommandLine_ParseInterval_ValidValue... PASSED
Running: CommandLine_ParseInterval_ZeroValue_UsesDefault... PASSED
Running: CommandLine_ParseMicrophoneFilter... PASSED
Running: CommandLine_ParseEventLogFlag... PASSED
Running: Logging_WriteLog_CreatesFile... PASSED
Running: Logging_WriteLog_ContainsMessage... PASSED
Running: Logging_WriteErrorLog_ContainsErrorPrefix... PASSED
Running: Volume_LastVolumeState_Tracking... PASSED
Running: Volume_VolumeChangeDetection... PASSED
Running: TestHelpers_FileOperations... PASSED

===============================================
Test Summary:
  Total tests: 10
  Passed: 10
  Failed: 0
===============================================
ALL TESTS PASSED!
```

## Adding New Tests

### Creating a Test Function

```cpp
TEST_FUNCTION(MyNewTest) {
    // Setup
    std::wstring testValue = L"test";
    
    // Execute
    bool result = SomeFunction(testValue);
    
    // Verify
    EXPECT_TRUE(result);
    EXPECT_EQ(testValue, L"test");
}
```

### Test Organization

1. **Group related tests**: Use descriptive prefixes (e.g., `CommandLine_`, `Logging_`)
2. **Follow AAA pattern**: Arrange, Act, Assert
3. **Use descriptive names**: Test names should describe what is being tested
4. **Test edge cases**: Include tests for boundary conditions and error cases

### Mock Objects

For testing complex functionality, use the provided mock objects:

```cpp
#include "MockAudioDevice.h"

TEST_FUNCTION(VolumeControl_MockDevice) {
    MockAudioDevice device(L"Test Device");
    device.mockVolume->SetTestVolume(0.5f);
    
    float volume = device.mockVolume->GetTestVolume();
    EXPECT_FLOAT_EQ(volume, 0.5f);
}
```

## Test Files

### Core Test Files

- `tests/SimpleTests.cpp`: Main test implementation
- `tests/SimpleTest.h`: Test framework definitions
- `tests/TestHelpers.h`: Utility functions for testing
- `tests/ServiceInterface.h`: Interface definitions for testable functions
- `tests/MockAudioDevice.h`: Mock objects for audio device testing

### Project Files

- `tests/SimpleTests.vcxproj`: Visual Studio project for simplified tests

### Scripts

- `run_tests.bat`: Main test runner script
- `quick_test.bat`: Quick test runner for specific categories

## Continuous Integration

Tests are automatically run in GitHub Actions on every commit. See `.github/workflows/build-check.yml` for the CI configuration.

## Troubleshooting

### Common Issues

1. **Build failures**: Ensure Visual Studio 2019/2022 with C++ tools is installed
2. **MSBuild not found**: Update the path in `run_tests.bat` to match your VS installation
3. **Test failures**: Check that you're running from the correct directory (project root)

### Debug Mode

To run tests in debug mode for troubleshooting:

```cmd
"C:\Program Files\Microsoft Visual Studio\2022\Community\Msbuild\Current\Bin\amd64\MSBuild.exe" tests\SimpleTests.vcxproj /p:Configuration=Debug /p:Platform=x64
tests\x64\Debug\SimpleTests\SimpleTests.exe
```

### Adding Debugging Output

```cpp
TEST_FUNCTION(MyDebugTest) {
    std::wcout << L"Debug: Testing value " << someValue << std::endl;
    EXPECT_TRUE(condition);
}
```

## Advanced Testing

### Integration Tests

Some tests require administrator privileges (service installation/uninstallation). These are marked as `DISABLED_` by default and can be enabled when running as administrator.

### Performance Tests

For performance testing, use the Windows performance counters or add timing to test functions:

```cpp
TEST_FUNCTION(Performance_LoggingSpeed) {
    DWORD startTime = GetTickCount();
    
    for (int i = 0; i < 1000; ++i) {
        WriteLog(L"Performance test message");
    }
    
    DWORD elapsed = GetTickCount() - startTime;
    EXPECT_LT(elapsed, 1000); // Should complete in less than 1 second
}
```
