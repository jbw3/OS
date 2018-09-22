#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

bool writeFile(const char* filename, int outfd)
{
    bool ok = false;

    int infd = open(filename, O_RDONLY);
    if (infd < 0)
    {
        dprintf(STDERR_FILENO, "cat: %s: No such file or directory\n", filename);
    }
    else
    {
        constexpr size_t BUFF_SIZE = 512;
        char buff[BUFF_SIZE];

        ssize_t numRead = read(infd, buff, BUFF_SIZE);
        while (numRead > 0)
        {
            write(outfd, buff, numRead);

            numRead = read(infd, buff, BUFF_SIZE);
        }

        close(infd);
    }

    return ok;
}

int main(int argc, const char* argv[])
{
    int rv = 0;

    int outfd = STDOUT_FILENO;
    for (int i = 1; i < argc; ++i)
    {
        rv |= writeFile(argv[i], outfd);
    }

    return rv;
}
