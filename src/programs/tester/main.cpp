#include "fcntl.h"
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

void ioTest(int argc, const char* argv[]);

void printCharLoop(int num);

void echoChar();

void fileTest();

int main(int argc, const char* argv[])
{
    // forkTest();
    // preemptTest();
    ioTest(argc, argv);
    // fileTest();

    return 0;
}

void forkTest()
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

    putchar('\n');
}

void ioTest(int argc, const char* argv[])
{
    if (argc <= 1)
    {
        return;
    }

    const char* paramBytes = argv[1];
    const char* finalByte = (argc >= 3) ? argv[2] : "m";

    printf("\x1b[%s%s", paramBytes, finalByte);
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

void readChar(int fd)
{
    char ch = '?';
    read(fd, &ch, 1);
    printf("pid: %i, char: %c\n", getpid(), ch);
}

void fileTest()
{
    dprintf(STDOUT_FILENO, "<%s>\n", "stdout");
    dprintf(STDERR_FILENO, "<%s>\n", "stderr");

    int fd = open("hello.txt", O_RDONLY);
    printf("fd = %i\n", fd);

    readChar(fd);

    pid_t pid = fork();
    if (pid == 0)
    {
        readChar(fd);
        close(fd);
        readChar(fd);
        exit(0);
    }

    wait(nullptr);

    readChar(fd);

    close(fd);
    readChar(fd);
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

        if (ch == '\r')
        {
            putchar('\n');
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
