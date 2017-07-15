#include "sched.h"
#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/wait.h"
#include "unistd.h"

void child(int num)
{
    char ch = 'A' + num;
    printf("child pid: %i, parent: %i\n", getpid(), getppid());

    for (int i = 0; i < 10; ++i)
    {
        putchar(ch);
        sched_yield();
    }
    putchar('\n');
}

int main()
{
    printf("init pid: %i\n", getpid());

    char ch;
    char str[32];
    char* ptr = str;
    for (int i = 0; i < 32; ++i)
    {
        ch = getchar();
        putchar(ch);

        if (ch == '\n')
        {
            break;
        }
        else
        {
            *(ptr++) = ch;
        }
    }

    *ptr = '\0';

    int numChildren = atoi(str);

    for (int i = 0; i < numChildren; ++i)
    {
        pid_t pid = fork();
        printf("%i, %i\n", getpid(), pid);
        if (pid < 0)
        {
            printf("Fork failed\n");
        }
        else if (pid == 0)
        {
            child(i);
            printf("%i is done\n", getpid());
            exit(0);
        }

        wait(nullptr);
    }

    // clean up child processes
    while (true)
    {
        pid_t pid = 0;
        do
        {
            pid = waitpid(-1, nullptr, WNOHANG);
        } while (pid > 0);

        sched_yield();
    }

    return 0;
}
