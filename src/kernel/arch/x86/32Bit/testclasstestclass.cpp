#include "unittests.h"

TestClassTestClass::TestClassTestClass() :
    TestClass("TestClass")
{
}

void TestClassTestClass::runTests()
{
    runTest("AssertTrue", []()
    {
        ASSERT_TRUE(false);
    });

    runTest("AssertFalse", []()
    {
        ASSERT_FALSE(false);
    });

    runTest("AssertEqual", []()
    {
        int i1 = 10;
        int i2 = 10;
        ASSERT_EQ(i1, i2);

        klog.logError("TEMP", "An error occurred. <&>");
    });

    runTest("AssertNotEqual", []()
    {
        int i1 = 401;
        int i2 = 1'003;
        ASSERT_NE(i1, i2);
    });

    runTest("AssertCStringEqual", []()
    {
        const char* str1 = "Hello";
        const char* str2 = "Hello!";
        ASSERT_CSTR_EQ(str1, str2);
    });

    runTest("AssertCStringNotEqual", []()
    {
        const char* str1 = "Hello";
        const char* str2 = "Hello";
        ASSERT_CSTR_NE(str1, str2);
    });
}
