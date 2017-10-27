#include <ctype.h>

int isalnum(int ch)
{
    return (ch >= 'A' && ch <= 'Z') ||
    (ch >= 'a' && ch <= 'z') ||
    (ch >= '0' && ch <= '9');
}

int isalpha(int ch)
{
    return (ch >= 'A' && ch <= 'Z') ||
    (ch >= 'a' && ch <= 'z');
}

int isdigit(int ch)
{
    return ch >= '0' && ch <= '9';
}

int isprint(int ch)
{
    return ch >= 0x20 && ch <= 0x7E;
}

int islower(int ch)
{
    return ch >= 'a' && ch <= 'z';
}

int isspace(int ch)
{
    return ch == ' '  ||
    ch == '\t' ||
    (ch >= '\n' && ch <= '\r');
}

int isupper(int ch)
{
    return ch >= 'A' && ch <= 'Z';
}

int isxdigit(int ch)
{
    return (ch >= 'A' && ch <= 'F') ||
           (ch >= 'a' && ch <= 'f') ||
           (ch >= '0' && ch <= '9');
}

int tolower(int ch)
{
    if (ch >= 'A' && ch <= 'Z')
    {
        return ch + ('a' - 'A');
    }
    return ch;
}

int toupper(int ch)
{
    if (ch >= 'a' && ch <= 'z')
    {
        return ch - ('a' - 'A');
    }
    return ch;
}
