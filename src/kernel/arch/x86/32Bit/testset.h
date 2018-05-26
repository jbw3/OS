#ifndef TEST_SET_H_
#define TEST_SET_H_

#include <stddef.h>

typedef void (*TestType)();

class TestSet
{
public:
    TestSet();

    void addTest(TestType test);

    TestSet& operator +=(TestType test);

    void run();

private:
    static constexpr size_t MAX_TESTS_SIZE = 32;
    TestType tests[MAX_TESTS_SIZE];
    size_t testsSize;
};

#endif // TEST_SET_H_
