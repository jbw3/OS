#include "stringutils.h"

char digitToChar(char digit, bool uppercase)
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
