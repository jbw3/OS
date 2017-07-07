#include "stddef.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"

int main()
{
    printf("init\npid: %i\n", getpid());

    return 0;
}
