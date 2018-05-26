#ifndef TEST_H_
#define TEST_H_

#include "testprinter.h"

#define FAIL(msg)                \
do                               \
{                                \
    TestPrinter::printFail(msg); \
    return;                      \
} while(false)

void runTest(const char* name, void (*test)());

#endif // TEST_H_
