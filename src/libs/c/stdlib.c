#include <ctype.h>
#include <stdlib.h>

int abs(int n)
{
    return n < 0 ? -n : n;
}

long labs(long n)
{
    return n < 0l ? -n : n;
}

long long llabs(long long n)
{
    return n < 0ll ? -n : n;
}

int atoi(const char* str)
{
    int num = 0;
    int idx = 0;
    char ch = str[idx];
    while (ch != '\0')
    {
        if (ch < '0' || ch > '9')
        {
            num = 0;
            break;
        }

        num *= 10;
        num += ch - '0';

        ++idx;
        ch = str[idx];
    }

    return num;
}

static char charToDigit(char ch)
{
    if (ch >= '0' && ch <= '9')
    {
        ch -= '0';
    }
    else if (ch >= 'a' && ch <= 'z')
    {
        ch -= 'a';
        ch += 10;
    }
    else if (ch >= 'A' && ch <= 'Z')
    {
        ch -= 'A';
        ch += 10;
    }

    return ch;
}

/**
 * @todo Check for num beyond max/min
 */
long strtol(const char* str, char** strEnd, int base)
{
    long num = 0;
    int idx = 0;
    int negative = 0;

    /* check if the base is invalid */
    if (base < 0 || base == 1 || base > 36)
    {
        if (strEnd != NULL)
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
            *strEnd = str;
#pragma GCC diagnostic pop
        }
        return 0;
    }

    /* skip leading whitespace */
    while (isspace(str[idx]))
    {
        ++idx;
    }

    if (str[idx] == '+')
    {
        ++idx;
    }
    else if (str[idx] == '-')
    {
        negative = 1;
        ++idx;
    }

    /* we don't have to check the length of str before accessing
    str[idx+1] because if this is out of range, str[idx] will be NUL */
    if (base == 16 && str[idx] == '0' && (str[idx+1] == 'x' || str[idx+1] == 'X'))
    {
        idx += 2;
    }
    /* if base is 0, figure out the base from the first chars */
    else if (base == 0)
    {
        if (str[idx] == '0' && (str[idx+1] == 'x' || str[idx+1] == 'X'))
        {
            base = 16;
            idx += 2;
        }
        else if (str[idx] == '0')
        {
            base = 8;
        }
        else
        {
            base = 10;
        }
    }

    char ch = str[idx];
    char digit = charToDigit(ch);

    /* check if the first char is invalid */
    if (digit == ch || digit >= base)
    {
        if (strEnd != NULL)
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
            *strEnd = str;
#pragma GCC diagnostic pop
        }
        return 0;
    }

    do
    {
        num *= base;
        num += digit;

        ++idx;
        ch = str[idx];
        digit = charToDigit(ch);
    } while (digit != ch && digit < base);

    if (negative)
    {
        num = -num;
    }

    if (strEnd != NULL)
    {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdiscarded-qualifiers"
        *strEnd = str + idx;
#pragma GCC diagnostic pop
    }

    return num;
}
