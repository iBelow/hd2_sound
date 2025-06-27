# Microphone Volume Service Usage Examples

## Basic Commands

### 1. Build Project
```cmd
# Via batch file (recommended)
build.bat

# Or directly via MSBuild
msbuild MicrophoneVolumeService.sln /p:Configuration=Release /p:Platform=x64
```

### 2. Install Service
```cmd
# Simple installation with default parameters
install_service.bat

# Or manually
MicrophoneVolumeService.exe -install
```

### 3. Test Without Installation
```cmd
# Via batch file
test_service.bat

# Or manually
MicrophoneVolumeService.exe -test
```

## Advanced Examples

### Installation with Settings

```cmd
# Check every 5 seconds for all microphones
MicrophoneVolumeService.exe -install -t 5

# Only for USB microphones every 3 seconds
MicrophoneVolumeService.exe -install -t 3 -m "USB"

# For specific microphone model
MicrophoneVolumeService.exe -install -t 2 -m "Blue Yeti"

# For laptop built-in microphone
MicrophoneVolumeService.exe -install -t 1 -m "Realtek"

# Use Windows Event Log (recommended - auto cleanup)
MicrophoneVolumeService.exe -install -t 2 -eventlog

# Use custom log file location
MicrophoneVolumeService.exe -install -t 2 -logfile "C:\MyLogs\microphone.log"

# Full example with all options
MicrophoneVolumeService.exe -install -t 3 -m "Blue Yeti" -eventlog
```

### Testing with Different Settings

```cmd
# Quick check every second
MicrophoneVolumeService.exe -test -t 1

# Only for specific microphone
MicrophoneVolumeService.exe -test -t 2 -m "Microphone Array"

# Search microphones by partial name
MicrophoneVolumeService.exe -test -t 3 -m "Audio"

# Test with Event Log (check Event Viewer for results)
MicrophoneVolumeService.exe -test -t 2 -eventlog

# Test with custom log file
MicrophoneVolumeService.exe -test -t 2 -logfile "C:\temp\test.log"
```

## Service Management

### Via Windows Command Line
```cmd
# Start service
net start MicrophoneVolumeService
sc start MicrophoneVolumeService

# Stop service
net stop MicrophoneVolumeService
sc stop MicrophoneVolumeService

# Check status
sc query MicrophoneVolumeService

# Configure startup
sc config MicrophoneVolumeService start= auto
sc config MicrophoneVolumeService start= demand
sc config MicrophoneVolumeService start= disabled
```

### Via PowerShell
```powershell
# Start service
Start-Service -Name "MicrophoneVolumeService"

# Stop service
Stop-Service -Name "MicrophoneVolumeService"

# Check status
Get-Service -Name "MicrophoneVolumeService"

# Restart service
Restart-Service -Name "MicrophoneVolumeService"
```

## Finding Microphones

To determine the exact name of your microphone:

1. **Via Windows Settings**:
   - Settings → System → Sound → Input
   - Copy device name

2. **Via Device Manager**:
   - Win+R → devmgmt.msc
   - Audio inputs and outputs
   - Find your microphone

3. **Via test mode**:
   ```cmd
   # Run without filter, log will show all found microphones
   MicrophoneVolumeService.exe -test
   ```

## Examples for Different Scenarios

### Gaming Setup
```cmd
# Fast correction for gaming microphone with Event Log
MicrophoneVolumeService.exe -install -t 1 -m "Gaming" -eventlog
```

### Office Work
```cmd
# Moderate frequency for built-in microphone
MicrophoneVolumeService.exe -install -t 5 -m "Internal" -eventlog
```

### Streaming/Recording
```cmd
# Frequent check for professional microphone
MicrophoneVolumeService.exe -install -t 2 -m "Blue Yeti" -eventlog
```

### Multiple Microphones
```cmd
# For all USB microphones with custom log
MicrophoneVolumeService.exe -install -t 3 -m "USB" -logfile "C:\Logs\usb_mics.log"

# For all microphones (no filter) with Event Log
MicrophoneVolumeService.exe -install -t 2 -eventlog
```

## Monitoring and Debugging

### View File Logs
```cmd
# Real-time (if using file logging)
powershell Get-Content "C:\Windows\Temp\MicrophoneVolumeService.log" -Wait

# Latest entries
powershell Get-Content "C:\Windows\Temp\MicrophoneVolumeService.log" -Tail 20
```

### View Windows Event Log
```cmd
# Open Event Viewer
eventvwr.msc

# Or via PowerShell (if using -eventlog option)
powershell Get-EventLog -LogName Application -Source "MicrophoneVolumeService" -Newest 10

# Filter only errors
powershell Get-EventLog -LogName Application -Source "MicrophoneVolumeService" -EntryType Error -Newest 5
```

### Log Behavior
- **File Logging**: Creates logs in specified file (default: C:\Windows\Temp\MicrophoneVolumeService.log)
- **Event Log**: Uses Windows Event Log with automatic rotation and cleanup
- **Smart Logging**: Only logs when microphone volume actually changes (not every interval)
- **Log Levels**: Information, Warning, Error events with proper categorization

## Logging Options and Recommendations

### Why the Logging was Improved
The original version logged "Volume set to 100%" every 2 seconds, which created massive log files. The new version only logs when:
- A new microphone is detected
- The microphone volume actually changes  
- There's an error or warning

### File vs Event Log

#### File Logging (Default)
```cmd
# Default location
MicrophoneVolumeService.exe -install -t 2

# Custom location  
MicrophoneVolumeService.exe -install -t 2 -logfile "D:\MyLogs\microphone.log"
```

**Pros:**
- Easy to access and read
- Can specify custom location
- Simple text format

**Cons:**
- Manual cleanup required
- Can grow large over time
- No automatic rotation

#### Windows Event Log (Recommended)
```cmd
# Use Event Log instead of file
MicrophoneVolumeService.exe -install -t 2 -eventlog
```

**Pros:**
- Automatic log rotation and cleanup by Windows
- Integrated with Windows logging system
- Better for production environments
- Can set retention policies
- Categorized by event type (Info/Warning/Error)

**Cons:**
- Requires Event Viewer to read easily
- Mixed with other system events

### Log Size Comparison

**Before (old version):**
- ~50 KB per hour (logging every 2 seconds)
- ~1.2 MB per day
- ~36 MB per month

**After (new version):**
- ~1-5 KB per day (only on actual changes)
- Typical: 10-20 log entries per day instead of 43,200
- **99% reduction in log volume**

### Recommended Settings

```cmd
# For personal gaming setup
MicrophoneVolumeService.exe -install -t 2 -eventlog

# For development/debugging
MicrophoneVolumeService.exe -install -t 1 -logfile "C:\debug\mic.log"

# For production servers
MicrophoneVolumeService.exe -install -t 5 -eventlog
```

## Automation

### Startup After Windows Installation
- Service is automatically configured for autostart on Windows boot
- Service starts immediately after installation (if installation succeeds)
- If service fails to start during installation, start manually:
  ```cmd
  # Via command line
  net start MicrophoneVolumeService
  
  # Via Services manager
  services.msc
  ```

### Deployment Script
```cmd
@echo off
echo Automatic service deployment...

REM Build
call build.bat
if %ERRORLEVEL% neq 0 exit /b 1

REM Install
call install_service.bat
if %ERRORLEVEL% neq 0 exit /b 1

echo Deployment completed!
```

## Troubleshooting

### Service Won't Start
```cmd
# Check service status
sc query MicrophoneVolumeService

# Check service configuration
sc qc MicrophoneVolumeService

# Check access rights
icacls "x64\Release\MicrophoneVolumeService.exe"

# Check dependencies
dumpbin /dependents "x64\Release\MicrophoneVolumeService.exe"

# Manual start
net start MicrophoneVolumeService

# Check last Windows error
net helpmsg [error_code]
```

### Service Doesn't Auto-Start After Reboot
```cmd
# Verify auto-start configuration
sc qc MicrophoneVolumeService

# Should show: START_TYPE: 2 AUTO_START
# If not, fix with:
sc config MicrophoneVolumeService start= auto

# Check if service is delayed start
sc qc MicrophoneVolumeService
# Look for DELAYED_AUTO_START_INFO

# Remove delayed start if present
sc config MicrophoneVolumeService start= auto
```

### Microphone Not Found
```cmd
# Test with verbose output
MicrophoneVolumeService.exe -test -t 5

# Check via PowerShell
powershell "Get-WmiObject -Class Win32_SoundDevice | Select Name, Status"
```

### Log Errors
- Check log file availability
- Ensure microphone name is correct
- Verify microphone is active and not used by another application

## 🧪 Testing Examples

### Running Tests

```cmd
# Build and run all tests
run_tests.bat

# Quick test for specific categories
quick_test.bat cmdline
quick_test.bat logging
quick_test.bat volume
```

### Manual Test Execution

```cmd
# Build tests manually
"C:\Program Files\Microsoft Visual Studio\2022\Community\Msbuild\Current\Bin\amd64\MSBuild.exe" tests\SimpleTests.vcxproj /p:Configuration=Release /p:Platform=x64

# Run tests manually
tests\x64\Release\SimpleTests\SimpleTests.exe
```

### Test Output Example

```
========================================
Microphone Volume Service Tests
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

### Adding New Tests

Create new test functions in `tests\SimpleTests.cpp`:

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

### Test Categories

- **CommandLine_**: Command line argument parsing tests
- **Logging_**: Logging system functionality tests  
- **Volume_**: Volume control and state tracking tests
- **TestHelpers_**: Utility and helper function tests

For detailed testing information, see [TESTING.md](TESTING.md).