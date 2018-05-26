#include "test.h"

const char* const Test::TEST_TAG = "Tests";
const char* Test::currentTestName = nullptr;

void Test::run(const char* name, void (*test)())
{
    currentTestName = name;

    test();

    currentTestName = nullptr;
}

void Test::fail(unsigned long long line, const char* msg)
{
    klog.logError(TEST_TAG, "{}, line {}: {}", currentTestName, line, msg);
}

bool Test::cStrComparison(unsigned long long line, const char* a, const char* b, bool equal, const char* aStr, const char* bStr)
{
    int rv = strcmp(a, b);
    if ( (equal && rv != 0) || (!equal && rv == 0) )
    {
        const char* compStr = equal ? "!=" : "==";
        klog.logError(TEST_TAG, "{}, line {}: {} {} {} (\"{}\" {} \"{}\")", currentTestName, line, aStr, compStr, bStr, a, compStr, b);
        return false;
    }
    return true;
}
