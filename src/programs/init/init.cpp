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
    pid_t pid = 0;

    pid = fork();
    if (pid < 0)
    {
        printf("Fork failed\n");
    }
    else if (pid == 0)
    {
        // forkTest();
        preemptTest();
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
    for (int i = 0; i < 2; ++i)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            printf("Fork failed\n");
        }
        else if (pid == 0)
        {
            if (i == 0) printCharLoop(i); else echoChar();
            exit(0);
        }
    }
}

void printCharLoop(int num)
{
    char ch = 'A' + num;

    while (true)
    {
        putchar(ch);

        // spin wait
        for (unsigned int i = 0; i < 50'000'000u; ++i);
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
