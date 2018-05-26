#ifndef TEST_CONTEXT_H_
#define TEST_CONTEXT_H_

#include "testexecutor.h"

class TestContext
{
public:
    friend TestExecutor;

    TestContext();

private:
    bool passed;
};

#endif // TEST_CONTEXT_H_
