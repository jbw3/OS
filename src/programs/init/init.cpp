#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/wait.h"
#include "unistd.h"

struct ShellInfo
{
    pid_t pid;
    int stdin;
    int stdout;
    int stderr;
};

pid_t startShell(int in, int out, int err);

int main()
{
    constexpr int SHELL_INFO_SIZE = 2;
    ShellInfo shellInfo[SHELL_INFO_SIZE] =
    {
        {-1, 0, 1, 2},
        {-1, 3, 3, 3}
    };

    // don't do anything if this process gets kicked
    // off more than once
    if (getpid() != 1)
    {
        exit(1);
    }

    for (int i = 0; i < SHELL_INFO_SIZE; ++i)
    {
        shellInfo[i].pid = startShell(shellInfo[i].stdin, shellInfo[i].stdout, shellInfo[i].stderr);
    }

    // clean up child processes
    while (true)
    {
        pid_t pid = wait(nullptr);

        // if a shell exited, start another one
        for (int i = 0; i < SHELL_INFO_SIZE; ++i)
        {
            if (pid == shellInfo[i].pid)
            {
                shellInfo[i].pid = startShell(shellInfo[i].stdin, shellInfo[i].stdout, shellInfo[i].stderr);
            }
        }
    }

    return 0;
}

pid_t startShell(int in, int out, int err)
{
    pid_t pid = fork();
    if (pid < 0)
    {
        printf("Fork failed\n");
    }
    else if (pid == 0)
    {
        // redirect stdin, stdout, and stderr
        dup2(in, STDIN_FILENO);
        dup2(out, STDOUT_FILENO);
        dup2(err, STDERR_FILENO);

        // close the extra serial port stream
        close(3);

        execl("sh", "sh", nullptr);

        // we shouldn't get here
        exit(-1);
    }

    return pid;
}
