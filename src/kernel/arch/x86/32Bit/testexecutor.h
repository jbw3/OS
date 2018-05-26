#ifndef TEST_EXECUTOR_H_
#define TEST_EXECUTOR_H_

#include <stddef.h>

class TestContext;

typedef void (*TestType)(TestContext& context);

class TestExecutor
{
public:
    static const char* TEST_TAG;

    TestExecutor();

    void runTest(const char* name, TestType test);

private:
    size_t total;
    size_t failed;
};

#endif // TEST_EXECUTOR_H_
