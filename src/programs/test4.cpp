#include "string.h"
#include "unistd.h"

int main()
{
    int x = 2;
    int y = 2;
    int z = x * y;

    const char str[] = "test4\n";
    write(STDOUT_FILENO, str, strlen(str));

    return z;
}
