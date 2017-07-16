#include "sched.h"
#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/wait.h"
#include "unistd.h"

int getNumber(const char* prompt);

void child();

void grandchild(int num);

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
        child();
        exit(0);
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

void child()
{
    while (true)
    {
        int numChildren = getNumber("Number of processes: ");

        for (int i = 0; i < numChildren; ++i)
        {
            pid_t pid = fork();
            if (pid < 0)
            {
                printf("Fork failed\n");
            }
            else if (pid == 0)
            {
                grandchild(i);
                exit(0);
            }
        }

        // wait for children to finish
        for (int i = 0; i < numChildren; ++i)
        {
            wait(nullptr);
        }
    }
}

void grandchild(int num)
{
    if (num == 0)
    {
        pid_t pid = fork();
        if (pid > 0)
        {
            exit(0);
        }
    }

    char ch = 'A' + num;

    for (int i = 0; i < 10; ++i)
    {
        putchar(ch);
        sched_yield();
    }
}

int getNumber(const char* prompt)
{
    printf("%s", prompt);

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

    return atoi(str);
}
