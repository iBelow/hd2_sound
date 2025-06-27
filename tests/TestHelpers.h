#pragma once
#include <windows.h>
#include <string>
#include <fstream>
#include <filesystem>
#include <vector>
#include <memory>
#include <map>

// Test helper functions
class TestHelpers {
public:
    // Create temporary test file
    static std::wstring CreateTempFile(const std::wstring& content = L"") {
        wchar_t tempDir[MAX_PATH];
        GetTempPath(MAX_PATH, tempDir);
        
        wchar_t tempFile[MAX_PATH];
        GetTempFileName(tempDir, L"MVS", 0, tempFile);
        
        if (!content.empty()) {
            std::wofstream file(tempFile);
            file << content;
            file.close();
        }
        
        return std::wstring(tempFile);
    }
    
    // Delete test file
    static void DeleteTempFile(const std::wstring& filePath) {
        DeleteFile(filePath.c_str());
    }
    
    // Read file content
    static std::wstring ReadFileContent(const std::wstring& filePath) {
        std::wifstream file(filePath);
        if (!file.is_open()) return L"";
        
        std::wstring content((std::istreambuf_iterator<wchar_t>(file)),
                             std::istreambuf_iterator<wchar_t>());
        return content;
    }
    
    // Check if file exists
    static bool FileExists(const std::wstring& filePath) {
        return std::filesystem::exists(filePath);
    }
    
    // Create mock command line arguments
    static void SetupMockArgs(const std::vector<std::wstring>& args, wchar_t**& argv, int& argc) {
        argc = static_cast<int>(args.size());
        argv = new wchar_t*[argc];
        
        for (int i = 0; i < argc; ++i) {
            size_t len = args[i].length() + 1;
            argv[i] = new wchar_t[len];
            wcscpy_s(argv[i], len, args[i].c_str());
        }
    }
    
    // Cleanup mock command line arguments
    static void CleanupMockArgs(wchar_t** argv, int argc) {
        if (argv) {
            for (int i = 0; i < argc; ++i) {
                delete[] argv[i];
            }
            delete[] argv;
        }
    }
    
    // Check if string contains substring
    static bool ContainsString(const std::wstring& haystack, const std::wstring& needle) {
        return haystack.find(needle) != std::wstring::npos;
    }
    
    // Get test data directory
    static std::wstring GetTestDataDir() {
        wchar_t currentDir[MAX_PATH];
        GetCurrentDirectory(MAX_PATH, currentDir);
        return std::wstring(currentDir) + L"\\TestData\\";
    }
    
    // Wait for file to be created (with timeout)
    static bool WaitForFile(const std::wstring& filePath, DWORD timeoutMs = 5000) {
        DWORD startTime = GetTickCount();
        while (GetTickCount() - startTime < timeoutMs) {
            if (FileExists(filePath)) {
                return true;
            }
            Sleep(100);
        }
        return false;
    }
    
    // Check if service is installed
    static bool IsServiceInstalled(const std::wstring& serviceName) {
        SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        if (!scm) return false;
        
        SC_HANDLE service = OpenService(scm, serviceName.c_str(), SERVICE_QUERY_STATUS);
        bool exists = (service != NULL);
        
        if (service) CloseServiceHandle(service);
        CloseServiceHandle(scm);
        
        return exists;
    }
    
    // Get service status
    static DWORD GetServiceStatus(const std::wstring& serviceName) {
        SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        if (!scm) return 0;
        
        SC_HANDLE service = OpenService(scm, serviceName.c_str(), SERVICE_QUERY_STATUS);
        if (!service) {
            CloseServiceHandle(scm);
            return 0;
        }
        
        SERVICE_STATUS status;
        DWORD currentState = 0;
        if (QueryServiceStatus(service, &status)) {
            currentState = status.dwCurrentState;
        }
        
        CloseServiceHandle(service);
        CloseServiceHandle(scm);
        
        return currentState;
    }
    
    // Clean up any test artifacts
    static void CleanupTestArtifacts() {
        // Remove any test log files
        std::vector<std::wstring> testFiles = {
            L"C:\\Windows\\Temp\\MicrophoneVolumeService_Test.log",
            L"C:\\Windows\\Temp\\TestLog.log"
        };
        
        for (const auto& file : testFiles) {
            DeleteFile(file.c_str());
        }
    }
};
