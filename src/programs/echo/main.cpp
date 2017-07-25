#include <stdio.h>

int main(int argc, const char* argv[])
{
    if (argc > 1)
    {
        printf("%s", argv[1]);
        for (int i = 2; i < argc; ++i)
        {
            printf(" %s", argv[i]);
        }
    }

    putchar('\n');

    return 0;
}
