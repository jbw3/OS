#include "testset.h"

TestSet::TestSet()
{
    testsSize = 0;
}

void TestSet::addTest(TestType test)
{
    if (testsSize >= MAX_TESTS_SIZE)
    {
        /// @todo print error message
    }
    else
    {
        tests[testsSize++] = test;
    }
}

TestSet& TestSet::operator +=(TestType test)
{
    addTest(test);
    return *this;
}

void TestSet::run()
{
    for (size_t i = 0; i < testsSize; ++i)
    {
        tests[i]();
    }
}
