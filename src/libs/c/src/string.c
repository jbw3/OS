#include <string.h>

int memcmp(const void* ptr1, const void* ptr2, size_t num)
{
    size_t i = 0;
    while (i < num)
    {
        int diff = ((const unsigned char*)ptr1)[i] - ((const unsigned char*)ptr2)[i];
        if (diff != 0)
        {
            return diff;
        }

        ++i;
    }

    return 0;
}

void* memcpy(void* dst, const void* src, size_t num)
{
    size_t i;
    for (i = 0; i < num; ++i)
    {
        ((char*)dst)[i] = ((const char*)src)[i];
    }

    return dst;
}

void* memmove(void* dst, const void* src, size_t num)
{
    size_t i;

    /* if the destination address is greater than the source
    address, copy "backward" in case the beginning of dst
    overlaps with the end of src */
    if (dst > src)
    {
        for (i = num; i > 0; --i)
        {
            ((char*)dst)[i - 1] = ((const char*)src)[i - 1];
        }
    }
    /* else copy "forward" in case the beginning of src
    overlaps with end end of dst */
    else
    {
        for (i = 0; i < num; ++i)
        {
            ((char*)dst)[i] = ((const char*)src)[i];
        }
    }

    return dst;
}

void* memset(void* ptr, int value, size_t num)
{
    size_t i;
    for (i = 0; i < num; ++i)
    {
        ((unsigned char*)ptr)[i] = (unsigned char)value;
    }

    return ptr;
}

char* strcat(char* str1, const char* str2)
{
    /* find the end of str1 */
    size_t idx1 = 0;
    while (str1[idx1] != '\0')
    {
        ++idx1;
    }

    /* copy str2 */
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

char* strncat(char* str1, const char* str2, size_t num)
{
    /* find the end of str1 */
    size_t idx1 = 0;
    while (str1[idx1] != '\0')
    {
        ++idx1;
    }

    /* copy str2 */
    size_t idx2 = 0;
    char ch;
    do
    {
        if (idx2 >= num)
        {
            str1[idx1] = '\0';
            break;
        }

        ch = str2[idx2];
        str1[idx1] = ch;
        ++idx1;
        ++idx2;
    } while (ch != '\0');

    return str1;
}

char* strchr(const char* str, int ch)
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
    char* ptr = str;
#pragma GCC diagnostic pop

    do
    {
        if (*ptr == (char)ch)
        {
            return ptr;
        }
    } while (*(ptr++) != '\0');

    return NULL;
}

int strcmp(const char* str1, const char* str2)
{
    size_t i = 0;
    char ch1 = str1[i];
    char ch2 = str2[i];
    while (ch1 != '\0' && ch2 != '\0')
    {
        int diff = ch1 - ch2;
        if (diff != 0)
        {
            return diff;
        }

        ++i;
        ch1 = str1[i];
        ch2 = str2[i];
    }

    return ch1 - ch2;
}

int strncmp(const char* str1, const char* str2, size_t num)
{
    size_t i = 0;
    char ch1 = str1[i];
    char ch2 = str2[i];
    while (ch1 != '\0' && ch2 != '\0')
    {
        int diff = ch1 - ch2;
        if (diff != 0)
        {
            return diff;
        }

        ++i;
        if (i >= num)
        {
            break;
        }

        ch1 = str1[i];
        ch2 = str2[i];
    }

    return ch1 - ch2;
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

char* strtok(char* str, const char* delimiters)
{
    static char* pos = NULL;
    char* start;

    if (str != NULL)
    {
        pos = str;
    }

    /* find the first char not in delimiters;
    this will be the start of the token */
    while (*pos != '\0' && strchr(delimiters, *pos) != NULL)
    {
        ++pos;
    }

    /* return a null pointer if a token could not be found */
    if (*pos == '\0')
    {
        return NULL;
    }

    start = pos;

    /* find a char in delimiters or a null char;
    this will be the end of the token */
    while (*pos != '\0' && strchr(delimiters, *pos) == NULL)
    {
        ++pos;
    }

    /* if the end if the string has not been found,
    insert a null char to mark the end of the token
    and increment pos to prepare for the next search */
    if (*pos != '\0')
    {
        *pos = '\0';
        ++pos;
    }

    return start;
}
