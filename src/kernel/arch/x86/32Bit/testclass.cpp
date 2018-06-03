#include <string.h>
#include "testclass.h"

const char* const TestClass::TEST_TAG = "Tests";
const char* TestClass::runningTestName = nullptr;

void TestClass::fail(unsigned long long line, const char* msg)
{
    klog.logError(TEST_TAG, "Fail: {}, line {}: {}", runningTestName, line, msg);
}

bool TestClass::cStrComparison(unsigned long long line, const char* a, const char* b, bool equal, const char* aStr, const char* bStr)
{
    int rv = strcmp(a, b);
    if ( (equal && rv != 0) || (!equal && rv == 0) )
    {
        const char* compStr = equal ? "!=" : "==";
        klog.logError(TEST_TAG, "Fail: {}, line {}: {} {} {} (\"{}\" {} \"{}\")", runningTestName, line, aStr, compStr, bStr, a, compStr, b);
        return false;
    }
    return true;
}

TestClass::TestClass(const char* className)
{
    strncpy(name, className, MAX_NAME_SIZE - 1);
    name[MAX_NAME_SIZE - 1] = '\0';
}

void TestClass::run()
{
    klog.logInfo(TEST_TAG, "TestClass: {}", name);

    runTests();
}

void TestClass::runTest(const char* testName, void (*test)())
{
    runningTestName = testName;

    klog.logInfo(TEST_TAG, "Test: {}", testName);

    test();

    runningTestName = nullptr;
}
