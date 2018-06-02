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

LoggerTestSuite::LoggerTestSuite() :
    TestSuite("Logger")
{
}

void LoggerTestSuite::runTests()
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
}
