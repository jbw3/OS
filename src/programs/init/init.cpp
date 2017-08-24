#include "sched.h"
#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/wait.h"
#include "unistd.h"

int main()
{
    // don't do anything if this process gets kicked
    // off more than once
    if (getpid() != 1)
    {
        exit(1);
    }

    pid_t pid = 0;

    pid = fork();
    if (pid < 0)
    {
        printf("Fork failed\n");
    }
    else if (pid == 0)
    {
        execl("sh", "sh", nullptr);

        // we shouldn't get here
        exit(-1);
    }

    // clean up child processes
    while (true)
    {
        wait(nullptr);
    }

    return 0;
}
