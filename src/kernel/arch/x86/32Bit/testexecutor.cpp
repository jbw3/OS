#include "kernellogger.h"
#include "testcontext.h"
#include "testexecutor.h"

const char* TestExecutor::TEST_TAG = "Tests";

TestExecutor::TestExecutor()
{
    total = 0;
    failed = 0;
}

void TestExecutor::runTest(const char* name, TestType test)
{
    TestContext context;
    test(context);

    ++total;
    if (!context.passed)
    {
        klog.logError(TEST_TAG, "{} failed!", name);
        ++failed;
    }
}
