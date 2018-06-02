#include <string.h>
#include "testsuite.h"

const char* const TestSuite::TEST_TAG = "Tests";
const char* TestSuite::runningTestName = nullptr;

void TestSuite::fail(unsigned long long line, const char* msg)
{
    klog.logError(TEST_TAG, "Fail: {}, line {}: {}", runningTestName, line, msg);
}

bool TestSuite::cStrComparison(unsigned long long line, const char* a, const char* b, bool equal, const char* aStr, const char* bStr)
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

TestSuite::TestSuite(const char* suiteName)
{
    strncpy(name, suiteName, MAX_NAME_SIZE - 1);
    name[MAX_NAME_SIZE - 1] = '\0';
}

void TestSuite::run()
{
    klog.logInfo(TEST_TAG, "TestSuite: {}", name);

    runTests();
}

void TestSuite::runTest(const char* testName, void (*test)())
{
    runningTestName = testName;

    klog.logInfo(TEST_TAG, "Test: {}", testName);

    test();

    runningTestName = nullptr;
}
