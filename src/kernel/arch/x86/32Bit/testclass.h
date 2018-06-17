#ifndef TEST_CLASS_H_
#define TEST_CLASS_H_

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

template<typename A, typename B>
bool cmpLt(const A& a, const B& b)
{
    return a < b;
}

template<typename A, typename B>
bool cmpLe(const A& a, const B& b)
{
    return a <= b;
}

template<typename A, typename B>
bool cmpGt(const A& a, const B& b)
{
    return a > b;
}

template<typename A, typename B>
bool cmpGe(const A& a, const B& b)
{
    return a >= b;
}

} // namespace details

class TestClass
{
private:
    static const char* const TEST_TAG;
    static const char* runningTestName;
    static bool runningTestPassed;

    static constexpr size_t MAX_NAME_SIZE = 64;
    char name[MAX_NAME_SIZE];
    size_t numTests;
    size_t numFailed;

    template<typename... Ts>
    static void failTest(const char* errorMsgFmt, Ts... ts)
    {
        runningTestPassed = false;
        klog.logError(TEST_TAG, errorMsgFmt, ts...);
    }

public:
    TestClass(const char* className);

    void run();

    size_t getNumTests() const
    {
        return numTests;
    }

    size_t getNumFailed() const
    {
        return numFailed;
    }

    static void fail(unsigned long long line, const char* msg1, const char* msg2 = nullptr);

    template<typename A, typename B>
    static bool comparison(unsigned long long line, const A& a, const B& b, bool (*cmp)(const A& x, const B& y), const char* aStr, const char* bStr, const char* compStr, const char* msg = nullptr)
    {
        if (!cmp(a, b))
        {
            if (msg == nullptr)
            {
                failTest("Fail: {}, line {}: {} {} {} ({} {} {})", runningTestName, line, aStr, compStr, bStr, a, compStr, b);
            }
            else
            {
                failTest("Fail: {}, line {}: {} {} {} ({} {} {}): {}", runningTestName, line, aStr, compStr, bStr, a, compStr, b, msg);
            }
            return false;
        }
        return true;
    }

    static bool cStrComparison(unsigned long long line, const char* a, const char* b, bool equal, const char* aStr, const char* bStr, const char* msg = nullptr);

protected:
    virtual void runTests() = 0;

    void runTest(const char* testName, void (*test)());
};

#define FAIL_BASE(evalFunc, ...)                  \
do                                                \
{                                                 \
    if (!(evalFunc))                              \
    {                                             \
        TestClass::fail(__LINE__, ##__VA_ARGS__); \
        return;                                   \
    }                                             \
} while (false)

#define FAIL(msg) \
FAIL_BASE(false, msg)

#define ASSERT_TRUE(a, ...) \
FAIL_BASE((a), #a" is false", ##__VA_ARGS__)

#define ASSERT_FALSE(a, ...) \
FAIL_BASE(!(a), #a" is true", ##__VA_ARGS__)

#define COMPARE_FAIL_BASE(a, b, cmp, compStr, ...)                                           \
do                                                                                           \
{                                                                                            \
    if (!TestClass::comparison(__LINE__, (a), (b), (cmp), #a, #b, (compStr), ##__VA_ARGS__)) \
    {                                                                                        \
        return;                                                                              \
    }                                                                                        \
} while (false)

#define ASSERT_EQ(a, b, ...) \
COMPARE_FAIL_BASE(a, b, details::cmpEq, "!=", ##__VA_ARGS__)

#define ASSERT_NE(a, b, ...) \
COMPARE_FAIL_BASE(a, b, details::cmpNe, "==", ##__VA_ARGS__)

#define ASSERT_LT(a, b, ...) \
COMPARE_FAIL_BASE(a, b, details::cmpLt, ">=", ##__VA_ARGS__)

#define ASSERT_LE(a, b, ...) \
COMPARE_FAIL_BASE(a, b, details::cmpLe, ">", ##__VA_ARGS__)

#define ASSERT_GT(a, b, ...) \
COMPARE_FAIL_BASE(a, b, details::cmpGt, "<=", ##__VA_ARGS__)

#define ASSERT_GE(a, b, ...) \
COMPARE_FAIL_BASE(a, b, details::cmpGe, "<", ##__VA_ARGS__)

#define COMPARE_FAIL_CSTR_BASE(a, b, equal, ...)                                        \
do                                                                                      \
{                                                                                       \
    if (!TestClass::cStrComparison(__LINE__, (a), (b), (equal), #a, #b, ##__VA_ARGS__)) \
    {                                                                                   \
        return;                                                                         \
    }                                                                                   \
} while (false)

#define ASSERT_CSTR_EQ(a, b, ...) \
COMPARE_FAIL_CSTR_BASE(a, b, true, ##__VA_ARGS__)

#define ASSERT_CSTR_NE(a, b, ...) \
COMPARE_FAIL_CSTR_BASE(a, b, false, ##__VA_ARGS__)

#endif // TEST_CLASS_H_
