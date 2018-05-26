#include "kernellogger.h"
#include "testprinter.h"

const char* const TestPrinter::TEST_TAG = "Tests";
const char* TestPrinter::currentTestName = nullptr;

void TestPrinter::setCurrentTestName(const char* name)
{
    currentTestName = name;
}

void TestPrinter::printFail(const char* msg)
{
    klog.logError(TEST_TAG, "{}: {}", currentTestName, msg);
}
