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
```

### Testing with Different Settings

```cmd
# Quick check every second
MicrophoneVolumeService.exe -test -t 1

# Only for specific microphone
MicrophoneVolumeService.exe -test -t 2 -m "Microphone Array"

# Search microphones by partial name
MicrophoneVolumeService.exe -test -t 3 -m "Audio"
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
# Fast correction for gaming microphone
MicrophoneVolumeService.exe -install -t 1 -m "Gaming"
```

### Office Work
```cmd
# Moderate frequency for built-in microphone
MicrophoneVolumeService.exe -install -t 5 -m "Internal"
```

### Streaming/Recording
```cmd
# Frequent check for professional microphone
MicrophoneVolumeService.exe -install -t 2 -m "Blue Yeti"
```

### Multiple Microphones
```cmd
# For all USB microphones
MicrophoneVolumeService.exe -install -t 3 -m "USB"

# For all microphones (no filter)
MicrophoneVolumeService.exe -install -t 2
```

## Monitoring and Debugging

### View Log
```cmd
# Real-time
powershell Get-Content "C:\Windows\Temp\MicrophoneVolumeService.log" -Wait

# Latest entries
powershell Get-Content "C:\Windows\Temp\MicrophoneVolumeService.log" -Tail 20
```

### Check Windows Events
```cmd
# Open Event Viewer
eventvwr.msc

# Or via PowerShell
powershell Get-EventLog -LogName Application -Source "MicrophoneVolumeService" -Newest 10
```

## Automation

### Startup After Windows Installation
Service is automatically configured for autostart after installation.

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
# Check access rights
icacls "x64\Release\MicrophoneVolumeService.exe"

# Check dependencies
dumpbin /dependents "x64\Release\MicrophoneVolumeService.exe"
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
