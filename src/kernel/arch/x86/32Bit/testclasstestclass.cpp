#include "unittests.h"

TestClassTestClass::TestClassTestClass() :
    TestClass("TestClass")
{
}

void TestClassTestClass::runTests()
{
    runTest("AssertTrue", []()
    {
        ASSERT_TRUE(true);
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
    });

    runTest("AssertNotEqual", []()
    {
        int i1 = 401;
        int i2 = 1'003;
        ASSERT_NE(i1, i2);
    });

    runTest("AssertLessThan", []()
    {
        int i1 = -24'309;
        int i2 = 108;
        ASSERT_LT(i1, i2);
    });

    runTest("AssertLessThanOrEqual", []()
    {
        int i1 = 401;
        int i2 = 1'003;
        ASSERT_LE(i1, i2);

        int i3 = 234;
        int i4 = 234;
        ASSERT_LE(i3, i4);
    });

    runTest("AssertGreaterThan", []()
    {
        int i1 = 1'203;
        int i2 = 455;
        ASSERT_GT(i1, i2);
    });

    runTest("AssertGreaterThanOrEqual", []()
    {
        int i1 = 43'209;
        int i2 = 2;
        ASSERT_GE(i1, i2);

        int i3 = -5;
        int i4 = -5;
        ASSERT_GE(i3, i4);
    });

    runTest("AssertCStringEqual", []()
    {
        const char* str1 = "Hello";
        const char* str2 = "Hello";
        ASSERT_CSTR_EQ(str1, str2);
    });

    runTest("AssertCStringNotEqual", []()
    {
        const char* str1 = "Hello";
        const char* str2 = "Hi";
        ASSERT_CSTR_NE(str1, str2);
    });
}
