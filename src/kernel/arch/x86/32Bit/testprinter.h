#ifndef TEST_PRINTER_H_
#define TEST_PRINTER_H_

class TestPrinter
{
public:
    static const char* const TEST_TAG;

    static void setCurrentTestName(const char* name);

    static void printFail(const char* msg);

private:
    static const char* currentTestName;
};

#endif // TEST_PRINTER_H_
