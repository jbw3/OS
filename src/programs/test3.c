#include "stddef.h"
#include "string.h"
#include "unistd.h"

int main()
{
    const char str[] = "Hi there!\n";

    for (int i = 0; i < 2; ++i)
    {
        write(STDOUT_FILENO, str, strlen(str));
    }

    return 0;
}
