#ifndef TEST_H_
#define TEST_H_

#include "testprinter.h"

#define FAIL(msg)                          \
do                                         \
{                                          \
    TestPrinter::printFail(__LINE__, msg); \
    return;                                \
} while(false)

#define ASSERT_BASE(evalFunc, msg)             \
do                                             \
{                                              \
    if (!(evalFunc))                           \
    {                                          \
        TestPrinter::printFail(__LINE__, msg); \
        return;                                \
    }                                          \
} while (false)

#define ASSERT_TRUE(a) \
ASSERT_BASE((a), #a" is false")

#define ASSERT_FALSE(a) \
ASSERT_BASE(!(a), #a" is true")

void runTest(const char* name, void (*test)());

#endif // TEST_H_
