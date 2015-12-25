#include <string.h>

void* memcpy(void* dst, const void* src, size_t num)
{
    for (size_t i = 0; i < num; ++i)
    {
        ((char*)dst)[i] = ((char*)src)[i];
    }

    return dst;
}

void* memset(void* ptr, int value, size_t num)
{
    for (size_t i = 0; i < num; ++i)
    {
        ((unsigned char*)ptr)[i] = (unsigned char)value;
    }

    return ptr;
}

char* strcat(char* str1, const char* str2)
{
    // find the end of str1
    size_t idx1 = 0;
    while (str1[idx1] != '\0')
    {
        ++idx1;
    }

    // copy str2
    size_t idx2 = 0;
    char ch;
    do
    {
        ch = str2[idx2];
        str1[idx1] = ch;
        ++idx1;
        ++idx2;
    } while (ch != '\0');

    return str1;
}

char* strcpy(char* dst, const char* src)
{
    size_t idx = 0;

    while ( (dst[idx] = src[idx]) != '\0')
    {
        ++idx;
    }

    return dst;
}

size_t strlen(const char* str)
{
    size_t size = 0;
    while (str[size] != '\0')
    {
        ++size;
    }

    return size;
}
