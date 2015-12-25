#include <stdlib.h>

int abs(int n)
{
    return n < 0 ? -n : n;
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

long labs(long n)
{
    return n < 0l ? -n : n;
}

long long llabs(long long n)
{
    return n < 0ll ? -n : n;
}
