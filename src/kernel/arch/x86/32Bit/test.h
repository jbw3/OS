#ifndef TEST_H_
#define TEST_H_

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

class Test
{
public:
    static const char* const TEST_TAG;

    static void run(const char* name, void (*test)());

    static void fail(unsigned long long line, const char* msg);

    template<typename A, typename B>
    static bool comparison(unsigned long long line, const A& a, const B& b, bool (*cmp)(const A& x, const B& y), const char* aStr, const char* bStr, const char* compStr)
    {
        if (!cmp(a, b))
        {
            klog.logError(TEST_TAG, "{}, line {}: {} {} {} ({} {} {})", currentTestName, line, aStr, compStr, bStr, a, compStr, b);
            return false;
        }
        return true;
    }

    static bool cStrComparison(unsigned long long line, const char* a, const char* b, bool equal, const char* aStr, const char* bStr);

private:
    static const char* currentTestName;
};

#define FAIL_BASE(evalFunc, msg)   \
do                                 \
{                                  \
    if (!(evalFunc))               \
    {                              \
        Test::fail(__LINE__, msg); \
        return;                    \
    }                              \
} while (false)

#define FAIL(msg) \
FAIL_BASE(false, msg)

#define ASSERT_TRUE(a) \
FAIL_BASE((a), #a" is false")

#define ASSERT_FALSE(a) \
FAIL_BASE(!(a), #a" is true")

#define COMPARE_FAIL_BASE(a, b, cmp, compStr)                            \
do                                                                       \
{                                                                        \
    if (!Test::comparison(__LINE__, (a), (b), (cmp), #a, #b, (compStr))) \
    {                                                                    \
        return;                                                          \
    }                                                                    \
} while (false)

#define ASSERT_EQ(a, b) \
COMPARE_FAIL_BASE(a, b, details::cmpEq, "!=")

#define ASSERT_NE(a, b) \
COMPARE_FAIL_BASE(a, b, details::cmpNe, "==")

#define COMPARE_FAIL_CSTR_BASE(a, b, equal)                         \
do                                                                  \
{                                                                   \
    if (!Test::cStrComparison(__LINE__, (a), (b), (equal), #a, #b)) \
    {                                                               \
        return;                                                     \
    }                                                               \
} while (false)

#define ASSERT_CSTR_EQ(a, b) \
COMPARE_FAIL_CSTR_BASE(a, b, true)

#define ASSERT_CSTR_NE(a, b) \
COMPARE_FAIL_CSTR_BASE(a, b, false)

#endif // TEST_H_
