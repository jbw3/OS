#include <string.h>
#include "testclass.h"

const char* const TestClass::TEST_TAG = "Tests";
const char* TestClass::runningTestName = nullptr;
bool TestClass::runningTestPassed = true;

void TestClass::fail(const char* filename, unsigned long long line, const char* msg1, const char* msg2)
{
    if (msg2 == nullptr)
    {
        failTest(runningTestName, filename, line, "{}", msg1);
    }
    else
    {
        failTest(runningTestName, filename, line, "{}: {}", msg1, msg2);
    }
}

bool TestClass::cStrComparison(const char* filename, unsigned long long line, const char* a, const char* b, bool equal, const char* aStr, const char* bStr, const char* msg)
{
    int rv = strcmp(a, b);
    if ( (equal && rv != 0) || (!equal && rv == 0) )
    {
        const char* compStr = equal ? "!=" : "==";
        if (msg == nullptr)
        {
            failTest(runningTestName, filename, line, "{} {} {} (\"{}\" {} \"{}\")", aStr, compStr, bStr, a, compStr, b);
        }
        else
        {
            failTest(runningTestName, filename, line, "{} {} {} (\"{}\" {} \"{}\"): {}", aStr, compStr, bStr, a, compStr, b, msg);
        }
        return false;
    }
    return true;
}

TestClass::TestClass(const char* className)
{
    strncpy(name, className, MAX_NAME_SIZE - 1);
    name[MAX_NAME_SIZE - 1] = '\0';

    numTests = 0;
    numFailed = 0;
}

void TestClass::run()
{
    klog.logInfo(TEST_TAG, "TestClass: {}", name);

    runTests();
}

void TestClass::runTest(const char* testName, void (*test)())
{
    runningTestName = testName;
    runningTestPassed = true;

    klog.logInfo(TEST_TAG, "Test: {}", testName);

    test();

    ++numTests;
    if (!runningTestPassed)
    {
        ++numFailed;
    }

    runningTestName = nullptr;
}
