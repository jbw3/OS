#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/wait.h"
#include "unistd.h"

pid_t startShell();

int main()
{
    // don't do anything if this process gets kicked
    // off more than once
    if (getpid() != 1)
    {
        exit(1);
    }

    pid_t shellPid = startShell();

    // clean up child processes
    while (true)
    {
        pid_t pid = wait(nullptr);

        // if the shell exited, start another one
        if (pid == shellPid)
        {
            shellPid = startShell();
        }
    }

    return 0;
}

pid_t startShell()
{
    pid_t pid = fork();
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

    return pid;
}
