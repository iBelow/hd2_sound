# Microphone Volume Control Service

![Version](https://img.shields.io/badge/version-1.0.2-blue)
![Platform](https://img.shields.io/badge/platform-Windows-lightgrey)
![License](https://img.shields.io/badge/license-MIT-green)
![Language](https://img.shields.io/badge/language-C++-blue)

![Microphone Volume Control Service](assets/img_1.png)

Windows service for automatic microphone volume management.

## What is this software for?

This service was specifically created to solve an annoying bug in the game **Helldivers 2**. The game has a problematic behavior where it automatically changes the microphone volume (sensitivity) settings in Windows, which causes issues with voice communication in Discord and other applications - your teammates can no longer hear you properly.

This mini-application continuously monitors and resets your microphone volume to 100% at regular intervals, ensuring that Helldivers 2 (or any other application) cannot permanently change your microphone settings. This way, your voice communication remains consistent and reliable during gaming sessions.

## Description

This service automatically sets the microphone volume level to 100% at specified time intervals. Uses Windows Core Audio API for audio device management.

## Features

- Automatic microphone volume setting to 100%
- Configurable check interval (default 2 seconds)
- Filter by specific microphone or work with all microphones
- Operation logging
- Runs as Windows system service

## Quick Start (Pre-built Release)

If you don't want to build from source, you can download a ready-to-use version:

1. **Download**: Go to [Releases](../../releases) and download the latest `MicrophoneVolumeService.exe`
2. **Install**: Run as administrator:
   ```cmd
   MicrophoneVolumeService.exe -install
   ```
3. **Verify**: The service starts automatically. To check status:
   ```cmd
   sc query MicrophoneVolumeService
   ```

For troubleshooting, check logs at: `C:\Windows\Temp\MicrophoneVolumeService.log`

**Note**: Administrator privileges are required for service installation and management.

## Build

1. Open `MicrophoneVolumeService.sln` in Visual Studio 2019/2022
2. Select Release configuration and x64 platform
3. Build the project (Build -> Build Solution)

Or use MSBuild from command line:

```cmd
msbuild MicrophoneVolumeService.sln /p:Configuration=Release /p:Platform=x64
```

## Usage

### Service Installation

```cmd
# Install with default parameters (2 sec interval, all microphones)
MicrophoneVolumeService.exe -install

# Install with interval setting (5 seconds)
MicrophoneVolumeService.exe -install -t 5

# Install with specific microphone filter
MicrophoneVolumeService.exe -install -t 3 -m "USB Microphone"

# Use Windows Event Log instead of file logging
MicrophoneVolumeService.exe -install -t 2 -eventlog
```

⚠️ **Note**: Service is configured for auto-start and starts immediately after installation.

### Service Status Check

```cmd
# Check if service is running and properly configured
check_service.bat
```

### Service Uninstallation

```cmd
MicrophoneVolumeService.exe -uninstall
```

### Testing (without service installation)

```cmd
# Test run with default parameters
MicrophoneVolumeService.exe -test

# Test run with interval setting
MicrophoneVolumeService.exe -test -t 1

# Test run with microphone filter
MicrophoneVolumeService.exe -test -t 2 -m "Realtek"
```

### Service Management

After installation, the service can be managed through:

1. **Windows Services Manager (services.msc)**:

   - Find "Microphone Volume Control Service"
   - Use Start/Stop/Restart buttons

2. **Command Line**:

   ```cmd
   # Start service
   net start "MicrophoneVolumeService"

   # Stop service
   net stop "MicrophoneVolumeService"

   # Or via sc
   sc start MicrophoneVolumeService
   sc stop MicrophoneVolumeService
   ```

## Command Line Parameters

- `-install` - Install service
- `-uninstall` - Uninstall service
- `-test` - Run in test mode (without service installation)
- `-version` - Show version information
- `-t <seconds>` - Check interval in seconds (default 2)
- `-m "<name>"` - Microphone name filter (default all microphones)

## Operation Log

The service maintains an operation log in the file:

```
C:\Windows\Temp\MicrophoneVolumeService.log
```

The log records:

- Service start and stop events
- Successful volume setting operations
- Audio device errors
- Information about found microphones

## Usage Examples

```cmd
# Install service to check every 5 seconds for all microphones
MicrophoneVolumeService.exe -install -t 5

# Install service only for USB microphone with 3-second checks
MicrophoneVolumeService.exe -install -t 3 -m "USB"

# Test with Realtek microphone every second
MicrophoneVolumeService.exe -test -t 1 -m "Realtek"
```

## Requirements

- Windows 10/11 or Windows Server 2016+
- Administrator privileges for service installation/uninstallation
- Visual Studio 2019/2022 with Windows SDK for building

## Troubleshooting

1. **Service won't start**: Check Windows Event Log (Event Viewer) and service log
2. **Can't find microphones**: Ensure microphones are active and properly configured in Windows
3. **Access errors**: Ensure service runs with necessary privileges

## Security

- Service runs under system account
- Uses only standard Windows APIs
- No network access required
- Log contains only audio device operation information
