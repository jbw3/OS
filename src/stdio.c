#include <stdarg.h>
#include <string.h>

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

static int writeInt(int num, char* str, int base, int uppercase)
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

int sprintf(char* buff, const char* fmt, ...)
{
    const char* buffStart = buff;
    int i;
    char* s;
    va_list args;
    va_start(args, fmt);

    while (*fmt != '\0')
    {
        char ch = *fmt;
        if (ch == '%')
        {
            switch (fmt[1])
            {
            /* output a literal % */
            case '%':
                *buff = '%';
                ++buff;
                fmt += 2;
                break;

            /* output a char */
            case 'c':
                i = va_arg(args, int);
                *buff = (char)(i);
                ++buff;
                fmt += 2;
                break;

            /* output a C-string */
            case 's':
                s = va_arg(args, char*);
                while (*s != '\0')
                {
                    *buff = *s;
                    ++buff;
                    ++s;
                }
                fmt += 2;
                break;

            /* output int */
            case 'd':
            case 'i':
                i = va_arg(args, int);
                buff += writeInt(i, buff, 10, 0);
                fmt += 2;
                break;

            /* if this is not a format sequence,
            simply output a % */
            default:
                *buff = ch;
                ++buff;
                ++fmt;
                break;
            }
        }
        else
        {
            *buff = ch;
            ++buff;
            ++fmt;
        }

    }
    *buff = '\0';

    va_end(args);

    return buff - buffStart;
}
