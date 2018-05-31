#include "test.h"

void runTestTests()
{
    Test::run("AssertTrue", []()
    {
        ASSERT_TRUE(true);
    });

    Test::run("AssertFalse", []()
    {
        ASSERT_FALSE(false);
    });

    Test::run("AssertEqual", []()
    {
        int i1 = 10;
        int i2 = 10;
        ASSERT_EQ(i1, i2);
    });

    Test::run("AssertNotEqual", []()
    {
        int i1 = 401;
        int i2 = 1'003;
        ASSERT_NE(i1, i2);
    });

    Test::run("AssertCStringEqual", []()
    {
        const char* str1 = "Hello";
        const char* str2 = "Hello";
        ASSERT_CSTR_EQ(str1, str2);
    });

    Test::run("AssertCStringNotEqual", []()
    {
        const char* str1 = "Hello";
        const char* str2 = "Hi";
        ASSERT_CSTR_NE(str1, str2);
    });
}
