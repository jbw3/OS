#ifndef TEST_PRINTER_H_
#define TEST_PRINTER_H_

#include "kernellogger.h"

class TestPrinter
{
public:
    static const char* const TEST_TAG;

    static void setCurrentTestName(const char* name);

    static void printFail(unsigned long long line, const char* msg);

    template<typename A, typename B>
    static bool testComparison(unsigned long long line, const A& a, const B& b, bool (*cmp)(const A& x, const B& y), const char* aStr, const char* bStr, const char* compStr)
    {
        if (!cmp(a, b))
        {
            klog.logError(TEST_TAG, "{}, line {}: {} {} {} ({} {} {})", currentTestName, line, aStr, compStr, bStr, a, compStr, b);
            return false;
        }
        return true;
    }

private:
    static const char* currentTestName;
};

#endif // TEST_PRINTER_H_
