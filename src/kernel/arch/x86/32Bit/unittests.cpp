#include "unittests.h"

void runUnitTests()
{
    TestSuiteTestSuite testSuiteSuite;
    testSuiteSuite.run();

    LoggerTestSuite loggerSuite;
    loggerSuite.run();
}
