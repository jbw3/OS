#include "test.h"
#include "testprinter.h"

void runTest(const char* name, void (*test)())
{
    TestPrinter::setCurrentTestName(name);

    test();

    TestPrinter::setCurrentTestName(nullptr);
}
