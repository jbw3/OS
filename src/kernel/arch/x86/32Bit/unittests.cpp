#include "unittests.h"

bool runUnitTests(size_t& numTests, size_t& numFailed)
{
    numTests = 0;
    numFailed = 0;

    TestClassTestClass testClassClass;
    testClassClass.run();
    numTests += testClassClass.getNumTests();
    numFailed += testClassClass.getNumFailed();

    LoggerTestClass loggerClass;
    loggerClass.run();
    numTests += loggerClass.getNumTests();
    numFailed += loggerClass.getNumFailed();

    return (numFailed == 0);
}
