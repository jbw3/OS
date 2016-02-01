#include <stdarg.h>
#include <string.h>

#include "stringutils.h"

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
                buff += writeSignedNum(i, buff, 10, 0);
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
