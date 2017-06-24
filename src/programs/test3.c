#include "stddef.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"

void print(const char* str)
{
    write(STDOUT_FILENO, str, strlen(str));
}

int main()
{
    const char str[] = "Hi there!\n";
    int rc = 123456;

    rc = write(STDOUT_FILENO, str, strlen(str));
    if (rc == 0)
    {
        print("rc = 0\n");
    }

    rc = write(STDERR_FILENO, str, strlen(str));
    if (rc == -1)
    {
        print("rc = -1\n");
    }

    pid_t pid = getpid();
    char pidBuff[32];
    sprintf(pidBuff, "pid: %i\n", pid);
    print(pidBuff);

    return 0;
}
