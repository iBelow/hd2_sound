#pragma once

// Simplified test framework for when Google Test is not available
#include <iostream>
#include <string>
#include <exception>

namespace SimpleTest {
    
class TestFailure : public std::exception {
private:
    std::string message;
public:
    TestFailure(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};

class TestRunner {
private:
    static int totalTests;
    static int passedTests;
    static int failedTests;
    
public:
    static void RunTest(const std::string& testName, void(*testFunc)()) {
        totalTests++;
        std::wcout << L"Running: " << std::wstring(testName.begin(), testName.end()) << L"...";
        
        try {
            testFunc();
            passedTests++;
            std::wcout << L" PASSED" << std::endl;
        } catch (const TestFailure& e) {
            failedTests++;
            std::wcout << L" FAILED" << std::endl;
            std::wcout << L"  Error: " << std::wstring(e.what(), e.what() + strlen(e.what())) << std::endl;
        } catch (const std::exception& e) {
            failedTests++;
            std::wcout << L" FAILED (Exception)" << std::endl;
            std::wcout << L"  Error: " << std::wstring(e.what(), e.what() + strlen(e.what())) << std::endl;
        } catch (...) {
            failedTests++;
            std::wcout << L" FAILED (Unknown exception)" << std::endl;
        }
    }
    
    static void PrintSummary() {
        std::wcout << L"\n===============================================" << std::endl;
        std::wcout << L"Test Summary:" << std::endl;
        std::wcout << L"  Total tests: " << totalTests << std::endl;
        std::wcout << L"  Passed: " << passedTests << std::endl;
        std::wcout << L"  Failed: " << failedTests << std::endl;
        std::wcout << L"===============================================" << std::endl;
        
        if (failedTests == 0) {
            std::wcout << L"ALL TESTS PASSED!" << std::endl;
        } else {
            std::wcout << L"SOME TESTS FAILED!" << std::endl;
        }
    }
    
    static int GetFailedCount() { return failedTests; }
};

// Simple assertion macros
#define EXPECT_TRUE(condition) \
    if (!(condition)) { \
        throw SimpleTest::TestFailure("Expected TRUE: " #condition); \
    }

#define EXPECT_FALSE(condition) \
    if ((condition)) { \
        throw SimpleTest::TestFailure("Expected FALSE: " #condition); \
    }

#define EXPECT_EQ(expected, actual) \
    if ((expected) != (actual)) { \
        throw SimpleTest::TestFailure("Expected equality: " #expected " == " #actual); \
    }

#define EXPECT_NE(expected, actual) \
    if ((expected) == (actual)) { \
        throw SimpleTest::TestFailure("Expected inequality: " #expected " != " #actual); \
    }

#define EXPECT_LT(val1, val2) \
    if (!((val1) < (val2))) { \
        throw SimpleTest::TestFailure("Expected less than: " #val1 " < " #val2); \
    }

#define EXPECT_LE(val1, val2) \
    if (!((val1) <= (val2))) { \
        throw SimpleTest::TestFailure("Expected less or equal: " #val1 " <= " #val2); \
    }

#define EXPECT_GT(val1, val2) \
    if (!((val1) > (val2))) { \
        throw SimpleTest::TestFailure("Expected greater than: " #val1 " > " #val2); \
    }

#define EXPECT_GE(val1, val2) \
    if (!((val1) >= (val2))) { \
        throw SimpleTest::TestFailure("Expected greater or equal: " #val1 " >= " #val2); \
    }

#define EXPECT_FLOAT_EQ(expected, actual) \
    if (abs((expected) - (actual)) > 0.0001f) { \
        throw SimpleTest::TestFailure("Expected float equality: " #expected " == " #actual); \
    }

#define EXPECT_NO_THROW(statement) \
    try { \
        statement; \
    } catch (...) { \
        throw SimpleTest::TestFailure("Expected no exception: " #statement); \
    }

#define EXPECT_FAILED(hr) \
    if (SUCCEEDED(hr)) { \
        throw SimpleTest::TestFailure("Expected HRESULT failure: " #hr); \
    }

// Test registration macro
#define TEST_FUNCTION(testName) \
    void testName(); \
    static bool testName##_registered = []() { \
        SimpleTest::TestRunner::RunTest(#testName, testName); \
        return true; \
    }(); \
    void testName()

// Initialize static members
int SimpleTest::TestRunner::totalTests = 0;
int SimpleTest::TestRunner::passedTests = 0;
int SimpleTest::TestRunner::failedTests = 0;

} // namespace SimpleTest
