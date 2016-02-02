#include <ctype.h>

int isspace(int ch)
{
    return ch == ' '  ||
           ch == '\t' ||
           (ch >= '\n' && ch <= '\r');
}

int isalpha(int ch)
{
    return (ch >= 'A' && ch <= 'Z') ||
           (ch >= 'a' && ch <= 'z');
}

int isupper(int ch)
{
    return ch >= 'A' && ch <= 'Z';
}

int islower(int ch)
{
    return ch >= 'a' && ch <= 'z';
}

int isdigit(int ch)
{
    return ch >= '0' && ch <= '9';
}

int isxdigit(int ch)
{
    return (ch >= 'A' && ch <= 'F') ||
           (ch >= 'a' && ch <= 'f') ||
           (ch >= '0' && ch <= '9');
}
