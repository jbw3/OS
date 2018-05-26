#ifndef TEST_CONTEXT_H_
#define TEST_CONTEXT_H_

#include "testset.h"

class TestContext
{
public:
    friend TestSet;

    TestContext();

private:
    bool passed;
};

#endif // TEST_CONTEXT_H_
