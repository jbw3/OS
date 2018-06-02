#include <string.h>
#include "testsuite.h"

const char* const TestSuite::TEST_TAG = "Tests";
const char* TestSuite::runningTestName = nullptr;
bool TestSuite::runningTestPassed = true;

void TestSuite::fail(unsigned long long line, const char* msg)
{
    klog.logError(TEST_TAG, "{}, line {}: {}", runningTestName, line, msg);
    runningTestPassed = false;
}

bool TestSuite::cStrComparison(unsigned long long line, const char* a, const char* b, bool equal, const char* aStr, const char* bStr)
{
    int rv = strcmp(a, b);
    if ( (equal && rv != 0) || (!equal && rv == 0) )
    {
        const char* compStr = equal ? "!=" : "==";
        klog.logError(TEST_TAG, "{}, line {}: {} {} {} (\"{}\" {} \"{}\")", runningTestName, line, aStr, compStr, bStr, a, compStr, b);
        runningTestPassed = false;
        return false;
    }
    return true;
}

TestSuite::TestSuite(const char* suiteName)
{
    strncpy(name, suiteName, MAX_NAME_SIZE - 1);
    name[MAX_NAME_SIZE - 1] = '\0';
}

void TestSuite::run()
{
    runTests();
}

void TestSuite::runTest(const char* testName, void (*test)())
{
    runningTestName = testName;
    runningTestPassed = true;

    test();

    if (runningTestPassed)
    {
        klog.logInfo(TEST_TAG, "{} passed.", runningTestName);
    }

    runningTestName = nullptr;
}
