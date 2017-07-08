#include "stddef.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"

int main()
{
    printf("init\npid: %i\n", getpid());

    printf("Before fork\n");

    pid_t pid = fork();

    printf("After fork. pid = %i\n", pid);

    return 0;
}
