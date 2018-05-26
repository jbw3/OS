#ifndef TEST_SET_H_
#define TEST_SET_H_

#include <stddef.h>

class TestContext;

typedef void (*TestType)(TestContext& context);

class TestSet
{
public:
    static const char* TEST_TAG;

    TestSet();

    void runTest(const char* name, TestType test);

private:
    size_t total;
    size_t failed;
};

#endif // TEST_SET_H_
