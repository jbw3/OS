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
