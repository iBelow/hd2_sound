#include <windows.h>
#include <winsvc.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <functiondiscoverykeys_devpkey.h>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <comdef.h>
#include <map>
#include "version.h"

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")

// Constants
#define SERVICE_NAME L"MicrophoneVolumeService"
#define SERVICE_DISPLAY_NAME L"Microphone Volume Control Service"
#define SERVICE_DESC L"Automatically sets microphone volume to 100%"

// Global variables
SERVICE_STATUS g_ServiceStatus = {0};
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE g_ServiceStopEvent = INVALID_HANDLE_VALUE;
DWORD g_IntervalSeconds = 2;           // Default interval
std::wstring g_MicrophoneFilter = L""; // Microphone filter
std::wstring g_LogFile = L"C:\\Windows\\Temp\\MicrophoneVolumeService.log";
HANDLE g_EventLogHandle = NULL;
std::map<std::wstring, float> g_LastVolumeState;  // Track last volume for each device
bool g_UseEventLog = false;  // Option to use Windows Event Log instead of file

// Functions for log management
void WriteLog(const std::wstring &message, WORD eventType = EVENTLOG_INFORMATION_TYPE)
{
    if (g_UseEventLog && g_EventLogHandle)
    {
        // Write to Windows Event Log
        LPCWSTR strings[] = { message.c_str() };
        ReportEventW(g_EventLogHandle, eventType, 0, 0, NULL, 1, 0, strings, NULL);
    }
    else
    {
        // Write to file
        std::wofstream logFile(g_LogFile, std::ios::app);
        if (logFile.is_open())
        {
            SYSTEMTIME st;
            GetLocalTime(&st);
            logFile << L"[" << st.wYear << L"-" 
                    << (st.wMonth < 10 ? L"0" : L"") << st.wMonth << L"-"
                    << (st.wDay < 10 ? L"0" : L"") << st.wDay << L" "
                    << (st.wHour < 10 ? L"0" : L"") << st.wHour << L":"
                    << (st.wMinute < 10 ? L"0" : L"") << st.wMinute << L":"
                    << (st.wSecond < 10 ? L"0" : L"") << st.wSecond
                    << L"] " << message << std::endl;
            logFile.close();
        }
    }
}

void WriteErrorLog(const std::wstring &message)
{
    WriteLog(L"ERROR: " + message, EVENTLOG_ERROR_TYPE);
}

void WriteWarningLog(const std::wstring &message)
{
    WriteLog(L"WARNING: " + message, EVENTLOG_WARNING_TYPE);
}

// Function to get current volume
float GetCurrentVolume(IMMDevice *pDevice)
{
    IAudioEndpointVolume *pEndpointVolume = NULL;
    float currentVolume = 0.0f;
    
    HRESULT hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void **)&pEndpointVolume);
    if (SUCCEEDED(hr))
    {
        hr = pEndpointVolume->GetMasterVolumeLevelScalar(&currentVolume);
        pEndpointVolume->Release();
    }
    
    return SUCCEEDED(hr) ? currentVolume : -1.0f;
}

// Function to set microphone volume
HRESULT SetMicrophoneVolume(IMMDevice *pDevice, float volume)
{
    IAudioEndpointVolume *pEndpointVolume = NULL;
    HRESULT hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void **)&pEndpointVolume);

    if (SUCCEEDED(hr))
    {
        hr = pEndpointVolume->SetMasterVolumeLevelScalar(volume, NULL);
        pEndpointVolume->Release();
    }

    return hr;
}

// Function to get device name
std::wstring GetDeviceName(IMMDevice *pDevice)
{
    IPropertyStore *pProps = NULL;
    PROPVARIANT varName;
    PropVariantInit(&varName);
    std::wstring deviceName = L"Unknown";

    HRESULT hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
    if (SUCCEEDED(hr))
    {
        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
        if (SUCCEEDED(hr))
        {
            deviceName = varName.pwszVal;
        }
        PropVariantClear(&varName);
        pProps->Release();
    }

    return deviceName;
}

// Main function for working with microphones
void ProcessMicrophones()
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        WriteErrorLog(L"COM initialization error: " + std::to_wstring(hr));
        return;
    }

    IMMDeviceEnumerator *pEnumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
                          __uuidof(IMMDeviceEnumerator), (void **)&pEnumerator);

    if (FAILED(hr))
    {
        WriteErrorLog(L"Device Enumerator creation error: " + std::to_wstring(hr));
        CoUninitialize();
        return;
    }

    IMMDeviceCollection *pCollection = NULL;
    hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pCollection);

    if (SUCCEEDED(hr))
    {
        UINT count;
        hr = pCollection->GetCount(&count);

        if (SUCCEEDED(hr))
        {
            // Only log device count on first run or when count changes
            static UINT lastDeviceCount = 0;
            static bool firstRun = true;
            
            if (firstRun || count != lastDeviceCount)
            {
                WriteLog(L"Active microphones found: " + std::to_wstring(count));
                lastDeviceCount = count;
                firstRun = false;
            }

            for (UINT i = 0; i < count; i++)
            {
                IMMDevice *pDevice = NULL;
                hr = pCollection->Item(i, &pDevice);

                if (SUCCEEDED(hr))
                {
                    std::wstring deviceName = GetDeviceName(pDevice);

                    // Check microphone filter
                    bool shouldProcess = g_MicrophoneFilter.empty() ||
                                         deviceName.find(g_MicrophoneFilter) != std::wstring::npos;

                    if (shouldProcess)
                    {
                        // Get current volume before setting
                        float currentVolume = GetCurrentVolume(pDevice);
                        const float targetVolume = 1.0f; // 100%
                        const float tolerance = 0.01f;   // 1% tolerance to avoid floating point issues
                        
                        // Check if volume has changed significantly
                        bool volumeChanged = false;
                        auto it = g_LastVolumeState.find(deviceName);
                        
                        if (it == g_LastVolumeState.end())
                        {
                            // First time seeing this device
                            g_LastVolumeState[deviceName] = currentVolume;
                            volumeChanged = true;
                            WriteLog(L"New microphone detected: " + deviceName + 
                                    L" (current volume: " + std::to_wstring((int)(currentVolume * 100)) + L"%)");
                        }
                        else if (std::abs(currentVolume - it->second) > tolerance)
                        {
                            // Volume has changed since last check
                            volumeChanged = true;
                            WriteLog(L"Volume changed for " + deviceName + 
                                    L": " + std::to_wstring((int)(it->second * 100)) + L"% -> " + 
                                    std::to_wstring((int)(currentVolume * 100)) + L"%");
                            g_LastVolumeState[deviceName] = currentVolume;
                        }

                        // Set volume to 100% if it's not already there
                        if (std::abs(currentVolume - targetVolume) > tolerance)
                        {
                            hr = SetMicrophoneVolume(pDevice, targetVolume);

                            if (SUCCEEDED(hr))
                            {
                                WriteLog(L"Volume corrected to 100% for: " + deviceName);
                                g_LastVolumeState[deviceName] = targetVolume;
                            }
                            else
                            {
                                WriteErrorLog(L"Volume setting error for " + deviceName +
                                             L": " + std::to_wstring(hr));
                            }
                        }
                        else if (volumeChanged)
                        {
                            // Volume was already at 100%, but we detected a device or want to log the state
                            WriteLog(L"Volume already at 100% for: " + deviceName);
                        }
                    }

                    pDevice->Release();
                }
            }
        }
        pCollection->Release();
    }
    else
    {
        WriteErrorLog(L"Audio devices enumeration error: " + std::to_wstring(hr));
    }

    pEnumerator->Release();
    CoUninitialize();
}

// Main service worker function
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
    WriteLog(L"Service started. Interval: " + std::to_wstring(g_IntervalSeconds) +
             L" sec. Filter: " + (g_MicrophoneFilter.empty() ? L"(all microphones)" : g_MicrophoneFilter));

    while (WaitForSingleObject(g_ServiceStopEvent, g_IntervalSeconds * 1000) == WAIT_TIMEOUT)
    {
        ProcessMicrophones();
    }

    WriteLog(L"Service stopped");
    return ERROR_SUCCESS;
}

// Service control function
VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
    switch (CtrlCode)
    {
    case SERVICE_CONTROL_STOP:
        WriteLog(L"Service stop signal received");
        if (g_ServiceStatus.dwCurrentState != SERVICE_STOP_PENDING)
        {
            g_ServiceStatus.dwControlsAccepted = 0;
            g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            g_ServiceStatus.dwWin32ExitCode = 0;
            g_ServiceStatus.dwCheckPoint = 4;

            if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
            {
                WriteLog(L"SetServiceStatus error");
            }

            SetEvent(g_ServiceStopEvent);
        }
        break;
    default:
        break;
    }
}

// Main service function
VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
    DWORD Status = E_FAIL;

    // Initialize Event Log handle if using Event Log
    if (g_UseEventLog)
    {
        g_EventLogHandle = RegisterEventSourceW(NULL, SERVICE_NAME);
        if (g_EventLogHandle == NULL)
        {
            // Fall back to file logging if Event Log registration fails
            g_UseEventLog = false;
            WriteLog(L"Warning: Could not register Event Log source, falling back to file logging");
        }
    }

    g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);
    if (g_StatusHandle == NULL)
    {
        return;
    }

    ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        WriteErrorLog(L"SetServiceStatus error");
    }

    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL)
    {
        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
        {
            WriteErrorLog(L"SetServiceStatus error");
        }
        return;
    }

    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        WriteErrorLog(L"SetServiceStatus error");
    }

    WriteLog(L"Microphone Volume Service started successfully");

    HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);
    if (hThread != NULL)
    {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }

    CloseHandle(g_ServiceStopEvent);

    // Clean up Event Log handle
    if (g_EventLogHandle)
    {
        DeregisterEventSource(g_EventLogHandle);
        g_EventLogHandle = NULL;
    }

    WriteLog(L"Microphone Volume Service stopped");

    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        WriteLog(L"SetServiceStatus error");
    }
}

// Service installation function
BOOL InstallService(DWORD intervalSeconds, const std::wstring &microphoneFilter, const std::wstring &logFile, bool useEventLog)
{
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (schSCManager == NULL)
    {
        std::wcout << L"Error opening Service Control Manager" << std::endl;
        return FALSE;
    }

    wchar_t szPath[MAX_PATH];
    if (!GetModuleFileName(NULL, szPath, MAX_PATH))
    {
        std::wcout << L"Error getting executable file path" << std::endl;
        CloseServiceHandle(schSCManager);
        return FALSE;
    }

    // Build parameter string
    std::wstring servicePath = szPath;
    servicePath += L" -service";
    if (intervalSeconds != 2)
    {
        servicePath += L" -t " + std::to_wstring(intervalSeconds);
    }
    if (!microphoneFilter.empty())
    {
        servicePath += L" -m \"" + microphoneFilter + L"\"";
    }
    if (useEventLog)
    {
        servicePath += L" -eventlog";
    }
    else if (!logFile.empty() && logFile != L"C:\\Windows\\Temp\\MicrophoneVolumeService.log")
    {
        servicePath += L" -logfile \"" + logFile + L"\"";
    }

    SC_HANDLE schService = CreateService(
        schSCManager,
        SERVICE_NAME,
        SERVICE_DISPLAY_NAME,
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,
        SERVICE_ERROR_NORMAL,
        servicePath.c_str(),
        NULL, NULL, NULL, NULL, NULL);

    if (schService == NULL)
    {
        DWORD error = GetLastError();
        if (error == ERROR_SERVICE_EXISTS)
        {
            std::wcout << L"Service already installed" << std::endl;
        }
        else
        {
            std::wcout << L"Service installation error: " << error << std::endl;
        }
        CloseServiceHandle(schSCManager);
        return FALSE;
    }

    // Set service description
    SERVICE_DESCRIPTION sd;
    sd.lpDescription = const_cast<LPWSTR>(SERVICE_DESC);
    ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &sd);

    // Start the service immediately after installation
    std::wcout << L"Service successfully installed" << std::endl;
    std::wcout << L"Logging: " << (useEventLog ? L"Windows Event Log" : (L"File: " + logFile)) << std::endl;
    
    if (StartService(schService, 0, NULL))
    {
        std::wcout << L"Service started successfully" << std::endl;
    }
    else
    {
        DWORD error = GetLastError();
        if (error == ERROR_SERVICE_ALREADY_RUNNING)
        {
            std::wcout << L"Service is already running" << std::endl;
        }
        else
        {
            std::wcout << L"Warning: Service installed but failed to start (error " << error << L")" << std::endl;
            std::wcout << L"You can start it manually with: net start MicrophoneVolumeService" << std::endl;
        }
    }

    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return TRUE;
}

// Service uninstallation function
BOOL UninstallService()
{
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (schSCManager == NULL)
    {
        std::wcout << L"Error opening Service Control Manager" << std::endl;
        return FALSE;
    }

    SC_HANDLE schService = OpenService(schSCManager, SERVICE_NAME, DELETE);
    if (schService == NULL)
    {
        std::wcout << L"Error opening service" << std::endl;
        CloseServiceHandle(schSCManager);
        return FALSE;
    }

    if (!DeleteService(schService))
    {
        std::wcout << L"Error deleting service" << std::endl;
        CloseServiceHandle(schService);
        CloseServiceHandle(schSCManager);
        return FALSE;
    }

    std::wcout << L"Service successfully uninstalled" << std::endl;
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return TRUE;
}

// Function to parse command line arguments
void ParseCommandLine(int argc, wchar_t *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if (wcscmp(argv[i], L"-t") == 0 && i + 1 < argc)
        {
            g_IntervalSeconds = _wtoi(argv[++i]);
            if (g_IntervalSeconds < 1)
                g_IntervalSeconds = 2;
        }
        else if (wcscmp(argv[i], L"-m") == 0 && i + 1 < argc)
        {
            g_MicrophoneFilter = argv[++i];
        }
        else if (wcscmp(argv[i], L"-logfile") == 0 && i + 1 < argc)
        {
            g_LogFile = argv[++i];
            g_UseEventLog = false;
        }
        else if (wcscmp(argv[i], L"-eventlog") == 0)
        {
            g_UseEventLog = true;
        }
    }
}

// Main function
int wmain(int argc, wchar_t *argv[])
{
    if (argc > 1)
    {
        if (wcscmp(argv[1], L"-install") == 0)
        {
            DWORD interval = 2;
            std::wstring filter = L"";
            std::wstring logFile = L"C:\\Windows\\Temp\\MicrophoneVolumeService.log";
            bool useEventLog = false;

            // Parse parameters for installation
            for (int i = 2; i < argc; i++)
            {
                if (wcscmp(argv[i], L"-t") == 0 && i + 1 < argc)
                {
                    interval = _wtoi(argv[++i]);
                    if (interval < 1)
                        interval = 2;
                }
                else if (wcscmp(argv[i], L"-m") == 0 && i + 1 < argc)
                {
                    filter = argv[++i];
                }
                else if (wcscmp(argv[i], L"-logfile") == 0 && i + 1 < argc)
                {
                    logFile = argv[++i];
                    useEventLog = false;
                }
                else if (wcscmp(argv[i], L"-eventlog") == 0)
                {
                    useEventLog = true;
                }
            }

            return InstallService(interval, filter, logFile, useEventLog) ? 0 : 1;
        }
        else if (wcscmp(argv[1], L"-uninstall") == 0)
        {
            return UninstallService() ? 0 : 1;
        }
        else if (wcscmp(argv[1], L"-service") == 0)
        {
            // Run as service
            ParseCommandLine(argc, argv);

            SERVICE_TABLE_ENTRY ServiceTable[] = {
                {const_cast<LPWSTR>(SERVICE_NAME), (LPSERVICE_MAIN_FUNCTION)ServiceMain},
                {NULL, NULL}};

            if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
            {
                WriteLog(L"StartServiceCtrlDispatcher error: " + std::to_wstring(GetLastError()));
                return GetLastError();
            }
            return 0;
        }
        else if (wcscmp(argv[1], L"-test") == 0)
        {
            // Test mode
            ParseCommandLine(argc, argv);
            std::wcout << L"Test mode. Interval: " << g_IntervalSeconds << L" sec." << std::endl;
            std::wcout << L"Microphone filter: " << (g_MicrophoneFilter.empty() ? L"(all)" : g_MicrophoneFilter) << std::endl;
            std::wcout << L"Logging: " << (g_UseEventLog ? L"Windows Event Log" : (L"File: " + g_LogFile)) << std::endl;
            std::wcout << L"Note: Only logs when volume actually changes" << std::endl;
            std::wcout << L"Press Ctrl+C to stop..." << std::endl;

            while (true)
            {
                ProcessMicrophones();
                std::this_thread::sleep_for(std::chrono::seconds(g_IntervalSeconds));
            }
            return 0;
        }
        else if (wcscmp(argv[1], L"-version") == 0 || wcscmp(argv[1], L"--version") == 0)
        {
            // Show version
            std::wcout << L"Microphone Volume Control Service" << std::endl;
            std::wcout << L"Version: " << VERSION_STRING << std::endl;
            std::wcout << L"Built for: Windows x64" << std::endl;
            std::wcout << L"Purpose: Fix Helldivers 2 microphone volume bug" << std::endl;
            return 0;
        }
    }

    // Show help
    std::wcout << L"Microphone Volume Control Service v" << VERSION_STRING_SHORT << std::endl;
    std::wcout << L"Created to fix Helldivers 2 microphone volume bug" << std::endl;
    std::wcout << L"" << std::endl;
    std::wcout << L"Usage:" << std::endl;
    std::wcout << L"  " << argv[0] << L" -install [-t seconds] [-m \"microphone_name\"] [-logfile path | -eventlog]" << std::endl;
    std::wcout << L"  " << argv[0] << L" -uninstall" << std::endl;
    std::wcout << L"  " << argv[0] << L" -test [-t seconds] [-m \"microphone_name\"] [-logfile path | -eventlog]" << std::endl;
    std::wcout << L"  " << argv[0] << L" -version" << std::endl;
    std::wcout << L"" << std::endl;
    std::wcout << L"Parameters:" << std::endl;
    std::wcout << L"  -t seconds     Check interval (default 2)" << std::endl;
    std::wcout << L"  -m name        Microphone name filter (default all)" << std::endl;
    std::wcout << L"  -logfile path  Log to custom file (default C:\\Windows\\Temp\\MicrophoneVolumeService.log)" << std::endl;
    std::wcout << L"  -eventlog      Use Windows Event Log instead of file" << std::endl;
    std::wcout << L"" << std::endl;
    std::wcout << L"Logging behavior:" << std::endl;
    std::wcout << L"  - Only logs when microphone volume actually changes" << std::endl;
    std::wcout << L"  - Reduces log file size and noise" << std::endl;
    std::wcout << L"  - Use Event Log for automatic log rotation" << std::endl;
    std::wcout << L"  -version       Show version information" << std::endl;
    std::wcout << L"" << std::endl;
    std::wcout << L"Examples:" << std::endl;
    std::wcout << L"  " << argv[0] << L" -install -t 5 -m \"USB Microphone\"" << std::endl;
    std::wcout << L"  " << argv[0] << L" -test -t 1" << std::endl;

    return 0;
}
