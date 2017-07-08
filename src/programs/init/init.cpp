#include "stddef.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"

int main()
{
    printf("init\npid: %i\n", getpid());

    printf("Before fork\n");

    pid_t rv = fork();

    printf("After fork. rv = %i\n", rv);
    printf("pid: %i\n", getpid());

    return 0;
}
