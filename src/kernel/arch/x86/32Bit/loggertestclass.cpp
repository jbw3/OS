#include "logger.h"
#include "unittests.h"

class TestLogger : public Logger
{
public:
    TestLogger();

    void reset();

    const char* getStr() const;

protected:
    void flush(const char* buff, size_t len) override;

private:
    static constexpr size_t MAX_STR_SIZE = 128;
    char str[MAX_STR_SIZE];
    size_t strSize;
    bool error;
};

TestLogger::TestLogger() :
    Logger(/*flushAfterMessage = */ true)
{
    reset();
}

void TestLogger::reset()
{
    str[0] = '\0';
    strSize = 0;
    error = false;
}

const char* TestLogger::getStr() const
{
    return str;
}

void TestLogger::flush(const char* buff, size_t len)
{
    if (!error)
    {
        if (strSize + len > MAX_STR_SIZE - 1)
        {
            const char* errorStr = "The TestLogger string buffer is not long enough!!!";
            strcpy(str, errorStr);
            strSize = strlen(errorStr);
            error = true;
        }
        else
        {
            memcpy(str + strSize, buff, len);
            strSize += len;
            str[strSize] = '\0';
        }
    }
}

LoggerTestClass::LoggerTestClass() :
    TestClass("Logger")
{
}

void LoggerTestClass::runTests()
{
    runTest("NoArgs", []()
    {
        const char* expectedStr = "This is a test.";

        TestLogger l;
        l.log(expectedStr);

        ASSERT_CSTR_EQ(expectedStr, l.getStr());
    });

    runTest("StringArg", []()
    {
        const char* expectedStr = "string: ABCD";
        const char* arg = "ABCD";

        TestLogger l;
        l.log("string: {}", arg);

        ASSERT_CSTR_EQ(expectedStr, l.getStr());
    });

    runTest("IntArg", []()
    {
        const char* expectedStr1 = "int: 450";
        int arg1 = 450;
        TestLogger l;

        l.log("int: 450", arg1);
        ASSERT_CSTR_EQ(expectedStr1, l.getStr());

        const char* expectedStr2 = "negative: -30";
        int arg2 = -30;
        l.reset();

        l.log("negative: -30", arg2);
        ASSERT_CSTR_EQ(expectedStr2, l.getStr());
    });

    runTest("UintArg", []()
    {
        const char* expectedStr1 = "unsigned int: 1034";
        unsigned int arg1 = 1'034;
        TestLogger l;

        l.log("unsigned int: 1034", arg1);
        ASSERT_CSTR_EQ(expectedStr1, l.getStr());

        const char* expectedStr2 = "(4000000000)";
        unsigned int arg2 = 4'000'000'000;
        l.reset();

        l.log("({})", arg2);
        ASSERT_CSTR_EQ(expectedStr2, l.getStr());
    });

    runTest("MultiArgTypes", []()
    {
        const char* expectedStr = "arg1 = 13008, arg2 = string, arg3 = !";
        int arg1 = 13'008;
        const char* arg2 = "string";
        char arg3 = '!';
        TestLogger l;

        l.log("arg1 = {}, arg2 = {}, arg3 = {}", arg1, arg2, arg3);
        ASSERT_CSTR_EQ(expectedStr, l.getStr());
    });

    runTest("TooManyArgs", []()
    {
        const char* expectedStr = "arg1 = 1, arg2 = 2";
        TestLogger l;

        l.log("arg1 = {}, arg2 = {}", 1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
        ASSERT_CSTR_EQ(expectedStr, l.getStr());
    });

    runTest("TooFewArgs", []()
    {
        const char* expectedStr = "arg1 = 1, arg2 = 2, arg3 = {}, arg4 = {}, arg5 = {}";
        TestLogger l;

        l.log("arg1 = {}, arg2 = {}, arg3 = {}, arg4 = {}, arg5 = {}", 1, 2);
        ASSERT_CSTR_EQ(expectedStr, l.getStr());
    });
}
