#ifndef TEST_H_
#define TEST_H_

#include "testprinter.h"

namespace details
{

template<typename A, typename B>
bool cmpEq(const A& a, const B& b)
{
    return a == b;
}

} // namespace details

#define FAIL_BASE(evalFunc, msg)               \
do                                             \
{                                              \
    if (!(evalFunc))                           \
    {                                          \
        TestPrinter::printFail(__LINE__, msg); \
        return;                                \
    }                                          \
} while (false)

#define FAIL(msg) \
FAIL_BASE(false, msg)

#define ASSERT_TRUE(a) \
FAIL_BASE((a), #a" is false")

#define ASSERT_FALSE(a) \
FAIL_BASE(!(a), #a" is true")

#define COMPARE_FAIL_BASE(a, b, cmp, compStr)                                       \
do                                                                                  \
{                                                                                   \
    if (!TestPrinter::testComparison(__LINE__, (a), (b), (cmp), #a, #b, (compStr))) \
    {                                                                               \
        return;                                                                     \
    }                                                                               \
} while (false)

#define ASSERT_EQ(a, b) \
COMPARE_FAIL_BASE(a, b, details::cmpEq, "!=")

void runTest(const char* name, void (*test)());

#endif // TEST_H_
