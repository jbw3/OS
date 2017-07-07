#include "stddef.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"

int main()
{
    printf("pid: %i\n", getpid());

    printf("Testing %i, %i, %i...\n", 1, 2, 3);

    return 0;
}
