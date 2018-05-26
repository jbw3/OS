#include "kernellogger.h"
#include "testcontext.h"
#include "testset.h"

const char* TEST_TAG = "Tests";

TestSet::TestSet()
{
    total = 0;
    failed = 0;
}

void TestSet::runTest(const char* name, TestType test)
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
