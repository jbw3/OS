#ifndef UNIT_TESTS_H_
#define UNIT_TESTS_H_

#include "testsuite.h"

class TestSuiteTestSuite : public TestSuite
{
public:
    TestSuiteTestSuite();

protected:
    void runTests() override;
};

class LoggerTestSuite : public TestSuite
{
public:
    LoggerTestSuite();

protected:
    void runTests() override;
};

void runUnitTests();

#endif // UNIT_TESTS_H_
