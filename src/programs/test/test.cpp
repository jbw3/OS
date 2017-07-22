#include "sched.h"
#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/wait.h"
#include "unistd.h"

int getNumber(const char* prompt);

void forkTest();

void printCharYield(int num);

void preemptTest();

void printCharLoop(int num);

void echoChar();

int main()
{
    // forkTest();
    preemptTest();

    return 0;
}

void forkTest()
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
                printCharYield(i);
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

void printCharYield(int num)
{
    char ch = 'A' + num;

    for (int i = 0; i < 10; ++i)
    {
        putchar(ch);
        sched_yield();
    }
}

void preemptTest()
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
                printCharLoop(i);
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

void printCharLoop(int num)
{
    char ch = 'A' + num;

    for (int i = 0; i < 160; ++i)
    {
        putchar(ch);

        // spin wait
        for (unsigned int i = 0; i < 1'000'000u; ++i);
    }
}

void echoChar()
{
    while (true)
    {
        char ch = getchar();
        putchar(ch);
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
