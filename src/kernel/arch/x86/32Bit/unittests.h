#ifndef UNIT_TESTS_H_
#define UNIT_TESTS_H_

#include "testclass.h"

class TestClassTestClass : public TestClass
{
public:
    TestClassTestClass();

protected:
    void runTests() override;
};

class LoggerTestClass : public TestClass
{
public:
    LoggerTestClass();

protected:
    void runTests() override;
};

bool runUnitTests(size_t& numTests, size_t& numFailed);

#endif // UNIT_TESTS_H_
