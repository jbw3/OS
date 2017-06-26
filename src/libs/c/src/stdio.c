#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "stringutils.h"

int sprintf(char* buff, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int rv = vsprintf(buff, fmt, args);

    va_end(args);

    return rv;
}

int vsprintf(char* buff, const char* fmt, va_list args)
{
    const char* buffStart = buff;
    int i;
    unsigned int ui;
    char* s;

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

            /* char */
            case 'c':
                i = va_arg(args, int);
                *buff = (char)(i);
                ++buff;
                fmt += 2;
                break;

            /* C-string */
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

            /* signed int */
            case 'd':
            case 'i':
                i = va_arg(args, int);
                buff += writeSignedNum(i, buff, 10, 0);
                fmt += 2;
                break;

            /* octal */
            case 'o':
                ui = va_arg(args, unsigned int);
                buff += writeUnsignedNum(ui, buff, 8, 0);
                fmt += 2;
                break;

            /* lowercase hexadecimal */
            case 'x':
                ui = va_arg(args, unsigned int);
                buff += writeUnsignedNum(ui, buff, 16, 0);
                fmt += 2;
                break;

            /* uppercase hexadecimal */
            case 'X':
                ui = va_arg(args, unsigned int);
                buff += writeUnsignedNum(ui, buff, 16, 1);
                fmt += 2;
                break;

            /* unsigned int */
            case 'u':
                ui = va_arg(args, unsigned int);
                buff += writeUnsignedNum(ui, buff, 10, 0);
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

    return buff - buffStart;
}
