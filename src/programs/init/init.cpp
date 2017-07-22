#include "sched.h"
#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/wait.h"
#include "unistd.h"

int main()
{
    pid_t pid = 0;

    pid = fork();
    if (pid < 0)
    {
        printf("Fork failed\n");
    }
    else if (pid == 0)
    {
        char* argv[1] = { nullptr };
        execv("test", argv);

        // we shouldn't get here
        exit(-1);
    }

    // clean up child processes
    while (true)
    {
        do
        {
            pid = waitpid(-1, nullptr, WNOHANG);
        } while (pid > 0);

        sched_yield();
    }

    return 0;
}
