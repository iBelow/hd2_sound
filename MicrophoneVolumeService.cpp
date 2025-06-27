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
DWORD g_IntervalSeconds = 2;  // Default interval
std::wstring g_MicrophoneFilter = L"";  // Microphone filter
std::wstring g_LogFile = L"C:\\Windows\\Temp\\MicrophoneVolumeService.log";

// Functions for log management
void WriteLog(const std::wstring& message) {
    std::wofstream logFile(g_LogFile, std::ios::app);
    if (logFile.is_open()) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        logFile << L"[" << st.wYear << L"-" << st.wMonth << L"-" << st.wDay 
                << L" " << st.wHour << L":" << st.wMinute << L":" << st.wSecond 
                << L"] " << message << std::endl;
        logFile.close();
    }
}

// Function to set microphone volume
HRESULT SetMicrophoneVolume(IMMDevice* pDevice, float volume) {
    IAudioEndpointVolume* pEndpointVolume = NULL;
    HRESULT hr = pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (void**)&pEndpointVolume);
    
    if (SUCCEEDED(hr)) {
        hr = pEndpointVolume->SetMasterVolumeLevelScalar(volume, NULL);
        pEndpointVolume->Release();
    }
    
    return hr;
}

// Function to get device name
std::wstring GetDeviceName(IMMDevice* pDevice) {
    IPropertyStore* pProps = NULL;
    PROPVARIANT varName;
    PropVariantInit(&varName);
    std::wstring deviceName = L"Unknown";
    
    HRESULT hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
    if (SUCCEEDED(hr)) {
        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varName);
        if (SUCCEEDED(hr)) {
            deviceName = varName.pwszVal;
        }
        PropVariantClear(&varName);
        pProps->Release();
    }
    
    return deviceName;
}

// Main function for working with microphones
void ProcessMicrophones() {
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        WriteLog(L"COM initialization error: " + std::to_wstring(hr));
        return;
    }

    IMMDeviceEnumerator* pEnumerator = NULL;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, 
                         __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    
    if (FAILED(hr)) {
        WriteLog(L"Device Enumerator creation error: " + std::to_wstring(hr));
        CoUninitialize();
        return;
    }

    IMMDeviceCollection* pCollection = NULL;
    hr = pEnumerator->EnumAudioEndpoints(eCapture, DEVICE_STATE_ACTIVE, &pCollection);
    
    if (SUCCEEDED(hr)) {
        UINT count;
        hr = pCollection->GetCount(&count);
        
        if (SUCCEEDED(hr)) {
            WriteLog(L"Active microphones found: " + std::to_wstring(count));
            
            for (UINT i = 0; i < count; i++) {
                IMMDevice* pDevice = NULL;
                hr = pCollection->Item(i, &pDevice);
                
                if (SUCCEEDED(hr)) {
                    std::wstring deviceName = GetDeviceName(pDevice);
                    
                    // Check microphone filter
                    bool shouldProcess = g_MicrophoneFilter.empty() || 
                                       deviceName.find(g_MicrophoneFilter) != std::wstring::npos;
                    
                    if (shouldProcess) {
                        hr = SetMicrophoneVolume(pDevice, 1.0f);  // 100% volume
                        
                        if (SUCCEEDED(hr)) {
                            WriteLog(L"Volume set to 100% for: " + deviceName);
                        } else {
                            WriteLog(L"Volume setting error for " + deviceName + 
                                   L": " + std::to_wstring(hr));
                        }
                    }
                    
                    pDevice->Release();
                }
            }
        }
        pCollection->Release();
    } else {
        WriteLog(L"Audio devices enumeration error: " + std::to_wstring(hr));
    }
    
    pEnumerator->Release();
    CoUninitialize();
}

// Main service worker function
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam) {
    WriteLog(L"Service started. Interval: " + std::to_wstring(g_IntervalSeconds) + 
             L" sec. Filter: " + (g_MicrophoneFilter.empty() ? L"(all microphones)" : g_MicrophoneFilter));
    
    while (WaitForSingleObject(g_ServiceStopEvent, g_IntervalSeconds * 1000) == WAIT_TIMEOUT) {
        ProcessMicrophones();
    }
    
    WriteLog(L"Service stopped");
    return ERROR_SUCCESS;
}

// Service control function
VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode) {
    switch (CtrlCode) {
    case SERVICE_CONTROL_STOP:
        WriteLog(L"Service stop signal received");
        if (g_ServiceStatus.dwCurrentState != SERVICE_STOP_PENDING) {
            g_ServiceStatus.dwControlsAccepted = 0;
            g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            g_ServiceStatus.dwWin32ExitCode = 0;
            g_ServiceStatus.dwCheckPoint = 4;

            if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) {
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
VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv) {
    DWORD Status = E_FAIL;

    g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);
    if (g_StatusHandle == NULL) {
        return;
    }

    ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) {
        WriteLog(L"SetServiceStatus error");
    }

    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL) {
        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) {
            WriteLog(L"SetServiceStatus error");
        }
        return;
    }

    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) {
        WriteLog(L"SetServiceStatus error");
    }

    HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);
    if (hThread != NULL) {
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
    }

    CloseHandle(g_ServiceStopEvent);

    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) {
        WriteLog(L"SetServiceStatus error");
    }
}

// Service installation function
BOOL InstallService(DWORD intervalSeconds, const std::wstring& microphoneFilter) {
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (schSCManager == NULL) {
        std::wcout << L"Error opening Service Control Manager" << std::endl;
        return FALSE;
    }

    wchar_t szPath[MAX_PATH];
    if (!GetModuleFileName(NULL, szPath, MAX_PATH)) {
        std::wcout << L"Error getting executable file path" << std::endl;
        CloseServiceHandle(schSCManager);
        return FALSE;
    }

    // Build parameter string
    std::wstring servicePath = szPath;
    servicePath += L" -service";
    if (intervalSeconds != 2) {
        servicePath += L" -t " + std::to_wstring(intervalSeconds);
    }
    if (!microphoneFilter.empty()) {
        servicePath += L" -m \"" + microphoneFilter + L"\"";
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
        NULL, NULL, NULL, NULL, NULL
    );

    if (schService == NULL) {
        DWORD error = GetLastError();
        if (error == ERROR_SERVICE_EXISTS) {
            std::wcout << L"Service already installed" << std::endl;
        } else {
            std::wcout << L"Service installation error: " << error << std::endl;
        }
        CloseServiceHandle(schSCManager);
        return FALSE;
    }

    // Set service description
    SERVICE_DESCRIPTION sd;
    sd.lpDescription = const_cast<LPWSTR>(SERVICE_DESC);
    ChangeServiceConfig2(schService, SERVICE_CONFIG_DESCRIPTION, &sd);

    std::wcout << L"Service successfully installed" << std::endl;
    CloseServiceHandle(schService);
    CloseServiceHandle(schSCManager);
    return TRUE;
}

// Service uninstallation function
BOOL UninstallService() {
    SC_HANDLE schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (schSCManager == NULL) {
        std::wcout << L"Error opening Service Control Manager" << std::endl;
        return FALSE;
    }

    SC_HANDLE schService = OpenService(schSCManager, SERVICE_NAME, DELETE);
    if (schService == NULL) {
        std::wcout << L"Error opening service" << std::endl;
        CloseServiceHandle(schSCManager);
        return FALSE;
    }

    if (!DeleteService(schService)) {
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
void ParseCommandLine(int argc, wchar_t* argv[]) {
    for (int i = 1; i < argc; i++) {
        if (wcscmp(argv[i], L"-t") == 0 && i + 1 < argc) {
            g_IntervalSeconds = _wtoi(argv[++i]);
            if (g_IntervalSeconds < 1) g_IntervalSeconds = 2;
        } else if (wcscmp(argv[i], L"-m") == 0 && i + 1 < argc) {
            g_MicrophoneFilter = argv[++i];
        }
    }
}

// Main function
int wmain(int argc, wchar_t* argv[]) {
    if (argc > 1) {
        if (wcscmp(argv[1], L"-install") == 0) {
            DWORD interval = 2;
            std::wstring filter = L"";
            
            // Parse parameters for installation
            for (int i = 2; i < argc; i++) {
                if (wcscmp(argv[i], L"-t") == 0 && i + 1 < argc) {
                    interval = _wtoi(argv[++i]);
                    if (interval < 1) interval = 2;
                } else if (wcscmp(argv[i], L"-m") == 0 && i + 1 < argc) {
                    filter = argv[++i];
                }
            }
            
            return InstallService(interval, filter) ? 0 : 1;
        } else if (wcscmp(argv[1], L"-uninstall") == 0) {
            return UninstallService() ? 0 : 1;
        } else if (wcscmp(argv[1], L"-service") == 0) {
            // Run as service
            ParseCommandLine(argc, argv);
            
            SERVICE_TABLE_ENTRY ServiceTable[] = {
                {const_cast<LPWSTR>(SERVICE_NAME), (LPSERVICE_MAIN_FUNCTION)ServiceMain},
                {NULL, NULL}
            };

            if (StartServiceCtrlDispatcher(ServiceTable) == FALSE) {
                WriteLog(L"StartServiceCtrlDispatcher error: " + std::to_wstring(GetLastError()));
                return GetLastError();
            }
            return 0;
        } else if (wcscmp(argv[1], L"-test") == 0) {
            // Test mode
            ParseCommandLine(argc, argv);
            std::wcout << L"Test mode. Interval: " << g_IntervalSeconds << L" sec." << std::endl;
            std::wcout << L"Microphone filter: " << (g_MicrophoneFilter.empty() ? L"(all)" : g_MicrophoneFilter) << std::endl;
            std::wcout << L"Press Ctrl+C to stop..." << std::endl;
            
            while (true) {
                ProcessMicrophones();
                std::this_thread::sleep_for(std::chrono::seconds(g_IntervalSeconds));
            }
            return 0;
        }
    }

    // Show help
    std::wcout << L"Usage:" << std::endl;
    std::wcout << L"  " << argv[0] << L" -install [-t seconds] [-m \"microphone_name\"]" << std::endl;
    std::wcout << L"  " << argv[0] << L" -uninstall" << std::endl;
    std::wcout << L"  " << argv[0] << L" -test [-t seconds] [-m \"microphone_name\"]" << std::endl;
    std::wcout << L"" << std::endl;
    std::wcout << L"Parameters:" << std::endl;
    std::wcout << L"  -t seconds     Check interval (default 2)" << std::endl;
    std::wcout << L"  -m name        Microphone name filter (default all)" << std::endl;
    std::wcout << L"" << std::endl;
    std::wcout << L"Examples:" << std::endl;
    std::wcout << L"  " << argv[0] << L" -install -t 5 -m \"USB Microphone\"" << std::endl;
    std::wcout << L"  " << argv[0] << L" -test -t 1" << std::endl;
    
    return 0;
}
