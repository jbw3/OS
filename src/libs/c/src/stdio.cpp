#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "stringutils.h"

extern "C"
{

int getchar()
{
    char ch;
    ssize_t numRead = -1;

    do
    {
        numRead = read(STDIN_FILENO, &ch, 1);
    } while (numRead == 0);

    if (numRead < 0)
    {
        return EOF;
    }

    return ch;
}

int putchar(int ch)
{
    unsigned char uChar = static_cast<unsigned char>(ch);
    ssize_t status = write(STDOUT_FILENO, &uChar, 1);

    return (status < 0) ? EOF : ch;
}

int puts(const char* s)
{
    size_t len = strlen(s);

    ssize_t status = write(STDOUT_FILENO, s, len);
    if (status < 0)
    {
        return EOF;
    }

    const char newline = '\n';
    status = write(STDOUT_FILENO, &newline, 1);
    if (status < 0)
    {
        return EOF;
    }

    return 0;
}

int printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int rv = vprintf(fmt, args);

    va_end(args);

    return rv;
}

int vprintf(const char* fmt, va_list args)
{
    char buff[128];

    int numChars = vsprintf(buff, fmt, args);

    ssize_t status = write(STDOUT_FILENO, buff, numChars);

    return (status < 0) ? -1 : numChars;
}

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
            // output a literal %
            case '%':
                *buff = '%';
                ++buff;
                fmt += 2;
                break;

            // char
            case 'c':
                i = va_arg(args, int);
                *buff = static_cast<char>(i);
                ++buff;
                fmt += 2;
                break;

            // C-string
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

            // signed int
            case 'd':
            case 'i':
                i = va_arg(args, int);
                buff += signedIntToString(i, buff, 10, false);
                fmt += 2;
                break;

            // octal
            case 'o':
                ui = va_arg(args, unsigned int);
                buff += unsignedIntToString(ui, buff, 8, false);
                fmt += 2;
                break;

            // lowercase hexadecimal
            case 'x':
                ui = va_arg(args, unsigned int);
                buff += unsignedIntToString(ui, buff, 16, false);
                fmt += 2;
                break;

            // uppercase hexadecimal
            case 'X':
                ui = va_arg(args, unsigned int);
                buff += unsignedIntToString(ui, buff, 16, true);
                fmt += 2;
                break;

            // unsigned int
            case 'u':
                ui = va_arg(args, unsigned int);
                buff += unsignedIntToString(ui, buff, 10, false);
                fmt += 2;
                break;

            // if this is not a format sequence,
            // simply output a %
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

} // extern "C"
