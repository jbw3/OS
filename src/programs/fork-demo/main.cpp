#include "sched.h"
#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/wait.h"
#include "unistd.h"

const char* NAMES[] = {
    "parent",
    "child",
    "grandchild",
    "great-grandchild",
};

void printInfo(int level);

int main()
{
    int level = 0;
    printInfo(level);

    while (level < 3)
    {
        pid_t pid = fork();

        // if fork failed...
        if (pid < 0)
        {
            printf("Fork failed\n");
            exit(1);
        }
        // if we're the parent...
        else if (pid > 0)
        {
            // wait for child to exit
            wait(nullptr);

            printf("I'm the %s process (%i). My child (%i) has exited.\n", NAMES[level], getpid(), pid);
            break;
        }
        else // we're the child
        {
            ++level;
            printInfo(level);
        }
    }

    return 0;
}

void printInfo(int level)
{
    printf("I'm the %s process (%i). My parent is %i.\n", NAMES[level], getpid(), getppid());
}
