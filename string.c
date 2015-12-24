#include <string.h>

void* memcpy(void* dst, const void* src, size_t num)
{
    for (size_t i = 0; i < num; ++i)
    {
        ((char*)dst)[i] = ((char*)src)[i];
    }
}

void* memset(void* ptr, int value, size_t num)
{
    for (size_t i = 0; i < num; ++i)
    {
        ((unsigned char*)ptr)[i] = (unsigned char)value;
    }
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
