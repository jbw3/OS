#ifndef TEST_SUITE_H_
#define TEST_SUITE_H_

#include <stddef.h>
#include "kernellogger.h"

namespace details
{

template<typename A, typename B>
bool cmpEq(const A& a, const B& b)
{
    return a == b;
}

template<typename A, typename B>
bool cmpNe(const A& a, const B& b)
{
    return a != b;
}

} // namespace details

class TestSuite
{
public:
    TestSuite(const char* suiteName);

    void run();

    static void fail(unsigned long long line, const char* msg);

    template<typename A, typename B>
    static bool comparison(unsigned long long line, const A& a, const B& b, bool (*cmp)(const A& x, const B& y), const char* aStr, const char* bStr, const char* compStr)
    {
        if (!cmp(a, b))
        {
            klog.logError(TEST_TAG, "{}, line {}: {} {} {} ({} {} {})", runningTestName, line, aStr, compStr, bStr, a, compStr, b);
            runningTestPassed = false;
            return false;
        }
        return true;
    }

    static bool cStrComparison(unsigned long long line, const char* a, const char* b, bool equal, const char* aStr, const char* bStr);

protected:
    virtual void runTests() = 0;

    void runTest(const char* testName, void (*test)());

private:
    static const char* const TEST_TAG;
    static const char* runningTestName;
    static bool runningTestPassed;

    static constexpr size_t MAX_NAME_SIZE = 64;
    char name[MAX_NAME_SIZE];

};

#define FAIL_BASE(evalFunc, msg)        \
do                                      \
{                                       \
    if (!(evalFunc))                    \
    {                                   \
        TestSuite::fail(__LINE__, msg); \
        return;                         \
    }                                   \
} while (false)

#define FAIL(msg) \
FAIL_BASE(false, msg)

#define ASSERT_TRUE(a) \
FAIL_BASE((a), #a" is false")

#define ASSERT_FALSE(a) \
FAIL_BASE(!(a), #a" is true")

#define COMPARE_FAIL_BASE(a, b, cmp, compStr)                                 \
do                                                                            \
{                                                                             \
    if (!TestSuite::comparison(__LINE__, (a), (b), (cmp), #a, #b, (compStr))) \
    {                                                                         \
        return;                                                               \
    }                                                                         \
} while (false)

#define ASSERT_EQ(a, b) \
COMPARE_FAIL_BASE(a, b, details::cmpEq, "!=")

#define ASSERT_NE(a, b) \
COMPARE_FAIL_BASE(a, b, details::cmpNe, "==")

#define COMPARE_FAIL_CSTR_BASE(a, b, equal)                              \
do                                                                       \
{                                                                        \
    if (!TestSuite::cStrComparison(__LINE__, (a), (b), (equal), #a, #b)) \
    {                                                                    \
        return;                                                          \
    }                                                                    \
} while (false)

#define ASSERT_CSTR_EQ(a, b) \
COMPARE_FAIL_CSTR_BASE(a, b, true)

#define ASSERT_CSTR_NE(a, b) \
COMPARE_FAIL_CSTR_BASE(a, b, false)

#endif // TEST_SUITE_H_
