#include <string.h>

#include "stringutils.h"

static char digitToChar(char digit, int uppercase)
{
    char ch;
    if (digit >= 10)
    {
        if (uppercase)
        {
            ch = digit + 'A' - 10;
        }
        else
        {
            ch = digit + 'a' - 10;
        }
    }
    else
    {
        ch = digit + '0';
    }
    return ch;
}

int writeSignedNum(long long num, char* str, int base, int uppercase)
{
    // need sizeof(int) chars for max signed number,
    // 1 char for possible negative sign,
    // and 1 char for null
    const int BUFF_SIZE = sizeof(num) + 2;
    char buff[sizeof(num) + 2];
    buff[BUFF_SIZE - 1] = '\0';

    int idx = BUFF_SIZE - 1;
    int negative = 1;
    if (num >= 0)
    {
        negative = 0;
        num = -num;
    }

    do
    {
        char digit = -(num % base);
        digit = digitToChar(digit, uppercase);
        buff[--idx] = digit;
        num /= base;
    } while (num < 0);

    if (negative)
    {
        buff[--idx] = '-';
    }

    strcpy(str, buff + idx);

    return (BUFF_SIZE - 1) - idx;
}

int writeUnsignedNum(unsigned long long num, char* str, int base, int uppercase)
{
    // need sizeof(int) chars for max signed number
    // and 1 char for null
    const int BUFF_SIZE = sizeof(num) + 1;
    char buff[sizeof(num) + 1];
    buff[BUFF_SIZE - 1] = '\0';

    int idx = BUFF_SIZE - 1;
    do
    {
        char digit = num % base;
        digit = digitToChar(digit, uppercase);
        buff[--idx] = digit;
        num /= base;
    } while (num > 0);

    strcpy(str, buff + idx);

    return (BUFF_SIZE - 1) - idx;
}
