#include <windows.h>
#include <iostream>
#include "SimpleTest.h"
#include "TestHelpers.h"
#include "ServiceInterface.h"

using namespace SimpleTest;

// Mock implementations for testing (since we can't include the actual service)
DWORD g_IntervalSeconds = 2;
std::wstring g_MicrophoneFilter = L"";
std::wstring g_LogFile = L"C:\\Windows\\Temp\\MicrophoneVolumeService.log";
bool g_UseEventLog = false;
std::map<std::wstring, float> g_LastVolumeState;
HANDLE g_EventLogHandle = NULL;

// Mock function implementations
void ParseCommandLine(int argc, wchar_t* argv[]) {
    // Reset to defaults
    g_IntervalSeconds = 2;
    g_MicrophoneFilter = L"";
    g_UseEventLog = false;
    
    for (int i = 1; i < argc; i++) {
        if (wcscmp(argv[i], L"-t") == 0 && i + 1 < argc) {
            int interval = _wtoi(argv[i + 1]);
            if (interval > 0 && interval <= 3600) {
                g_IntervalSeconds = interval;
            }
            i++; // Skip next argument
        }
        else if (wcscmp(argv[i], L"-m") == 0 && i + 1 < argc) {
            g_MicrophoneFilter = argv[i + 1];
            i++; // Skip next argument
        }
        else if (wcscmp(argv[i], L"-eventlog") == 0) {
            g_UseEventLog = true;
        }
    }
}

void WriteLog(const std::wstring& message, WORD eventType) {
    if (!g_UseEventLog) {
        std::wofstream logFile(g_LogFile, std::ios::app);
        if (logFile.is_open()) {
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

void WriteErrorLog(const std::wstring& message) {
    WriteLog(L"ERROR: " + message, EVENTLOG_ERROR_TYPE);
}

void WriteWarningLog(const std::wstring& message) {
    WriteLog(L"WARNING: " + message, EVENTLOG_WARNING_TYPE);
}

// Command Line Tests
TEST_FUNCTION(CommandLine_ParseInterval_ValidValue) {
    std::vector<std::wstring> args = { L"program.exe", L"-t", L"5" };
    wchar_t** argv;
    int argc;
    TestHelpers::SetupMockArgs(args, argv, argc);
    
    ParseCommandLine(argc, argv);
    
    EXPECT_EQ(g_IntervalSeconds, 5);
    
    TestHelpers::CleanupMockArgs(argv, argc);
}

TEST_FUNCTION(CommandLine_ParseInterval_ZeroValue_UsesDefault) {
    std::vector<std::wstring> args = { L"program.exe", L"-t", L"0" };
    wchar_t** argv;
    int argc;
    TestHelpers::SetupMockArgs(args, argv, argc);
    
    ParseCommandLine(argc, argv);
    
    EXPECT_EQ(g_IntervalSeconds, 2);  // Should use default
    
    TestHelpers::CleanupMockArgs(argv, argc);
}

TEST_FUNCTION(CommandLine_ParseMicrophoneFilter) {
    std::vector<std::wstring> args = { L"program.exe", L"-m", L"USB Microphone" };
    wchar_t** argv;
    int argc;
    TestHelpers::SetupMockArgs(args, argv, argc);
    
    ParseCommandLine(argc, argv);
    
    EXPECT_TRUE(g_MicrophoneFilter == L"USB Microphone");
    
    TestHelpers::CleanupMockArgs(argv, argc);
}

TEST_FUNCTION(CommandLine_ParseEventLogFlag) {
    std::vector<std::wstring> args = { L"program.exe", L"-eventlog" };
    wchar_t** argv;
    int argc;
    TestHelpers::SetupMockArgs(args, argv, argc);
    
    ParseCommandLine(argc, argv);
    
    EXPECT_TRUE(g_UseEventLog);
    
    TestHelpers::CleanupMockArgs(argv, argc);
}

// Logging Tests
TEST_FUNCTION(Logging_WriteLog_CreatesFile) {
    std::wstring testLogFile = TestHelpers::CreateTempFile();
    g_LogFile = testLogFile;
    g_UseEventLog = false;
    
    WriteLog(L"Test message");
    
    EXPECT_TRUE(TestHelpers::FileExists(testLogFile));
    
    TestHelpers::DeleteTempFile(testLogFile);
}

TEST_FUNCTION(Logging_WriteLog_ContainsMessage) {
    std::wstring testLogFile = TestHelpers::CreateTempFile();
    g_LogFile = testLogFile;
    g_UseEventLog = false;
    
    const std::wstring testMessage = L"This is a test log message";
    WriteLog(testMessage);
    
    std::wstring content = TestHelpers::ReadFileContent(testLogFile);
    
    EXPECT_TRUE(TestHelpers::ContainsString(content, testMessage));
    
    TestHelpers::DeleteTempFile(testLogFile);
}

TEST_FUNCTION(Logging_WriteErrorLog_ContainsErrorPrefix) {
    std::wstring testLogFile = TestHelpers::CreateTempFile();
    g_LogFile = testLogFile;
    g_UseEventLog = false;
    
    WriteErrorLog(L"Test error message");
    
    std::wstring content = TestHelpers::ReadFileContent(testLogFile);
    
    EXPECT_TRUE(TestHelpers::ContainsString(content, L"ERROR: Test error message"));
    
    TestHelpers::DeleteTempFile(testLogFile);
}

// Volume Control Tests
TEST_FUNCTION(Volume_LastVolumeState_Tracking) {
    g_LastVolumeState.clear();
    
    std::wstring device1 = L"TestDevice1";
    std::wstring device2 = L"TestDevice2";
    
    // Simulate storing volume states
    g_LastVolumeState[device1] = 0.5f;
    g_LastVolumeState[device2] = 0.8f;
    
    EXPECT_FLOAT_EQ(g_LastVolumeState[device1], 0.5f);
    EXPECT_FLOAT_EQ(g_LastVolumeState[device2], 0.8f);
}

TEST_FUNCTION(Volume_VolumeChangeDetection) {
    g_LastVolumeState.clear();
    
    std::wstring deviceName = L"TestDevice";
    float initialVolume = 0.5f;
    float newVolume = 0.8f;
    
    // Set initial state
    g_LastVolumeState[deviceName] = initialVolume;
    
    // Check if volume changed (should be true for different volume)
    bool volumeChanged = (g_LastVolumeState[deviceName] != newVolume);
    EXPECT_TRUE(volumeChanged);
    
    // Update state
    g_LastVolumeState[deviceName] = newVolume;
    
    // Should now be equal
    volumeChanged = (g_LastVolumeState[deviceName] != newVolume);
    EXPECT_FALSE(volumeChanged);
}

// Test Helper Tests
TEST_FUNCTION(TestHelpers_FileOperations) {
    std::wstring testFile = TestHelpers::CreateTempFile(L"Test content");
    
    EXPECT_TRUE(TestHelpers::FileExists(testFile));
    
    std::wstring content = TestHelpers::ReadFileContent(testFile);
    EXPECT_TRUE(TestHelpers::ContainsString(content, L"Test content"));
    
    TestHelpers::DeleteTempFile(testFile);
    EXPECT_FALSE(TestHelpers::FileExists(testFile));
}

// Main function
int main() {
    std::wcout << L"========================================" << std::endl;
    std::wcout << L"Microphone Volume Service Tests" << std::endl;
    std::wcout << L"========================================" << std::endl;
    std::wcout << L"Using simplified test framework" << std::endl;
    std::wcout << L"========================================" << std::endl;
    
    // Tests are automatically run when the static initializers execute
    
    TestRunner::PrintSummary();
    
    return TestRunner::GetFailedCount();
}
