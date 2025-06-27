#pragma once
#include <windows.h>
#include <string>
#include <map>

// Forward declarations for functions to test
// These functions need to be exposed from the main service

// Command line parsing
void ParseCommandLine(int argc, wchar_t* argv[]);

// Logging functions  
void WriteLog(const std::wstring& message, WORD eventType = EVENTLOG_INFORMATION_TYPE);
void WriteErrorLog(const std::wstring& message);
void WriteWarningLog(const std::wstring& message);

// Service utility functions (these would need COM/Audio includes for full implementation)
//HRESULT SetMicrophoneVolume(IMMDevice* pDevice, float volume);
//float GetCurrentVolume(IMMDevice* pDevice);

BOOL InstallService();
BOOL UninstallService();
void ServiceMain(DWORD argc, LPTSTR* argv);
void ServiceCtrlHandler(DWORD dwCtrl);

// Global variables that we need to test
extern DWORD g_IntervalSeconds;
extern std::wstring g_MicrophoneFilter;
extern std::wstring g_LogFile;
extern bool g_UseEventLog;
extern std::map<std::wstring, float> g_LastVolumeState;
extern HANDLE g_EventLogHandle;
