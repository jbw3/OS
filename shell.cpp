#include "screen.h"
#include "shell.h"

Shell::Shell()
{
}

void Shell::update()
{
    char ch;
    bool avail = screen.read(ch);
    while (avail)
    {
        screen << ch;

        avail = screen.read(ch);
    }
}
