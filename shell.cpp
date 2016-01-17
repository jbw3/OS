#include "screen.h"
#include "shell.h"
#include "string.h"

Shell::Shell()
{
    prompt();
}

void Shell::update()
{
    char ch;
    bool avail = screen.read(ch);
    while (avail)
    {
        screen << ch;

        processChar(ch);

        avail = screen.read(ch);
    }
}

void Shell::prompt()
{
    screen << "> ";
}

void Shell::processChar(char ch)
{
    if (ch == '\n')
    {
        cmd[cmdIdx] = '\0';
        processCmd();
    }
    else if (cmdIdx == CMD_MAX_SIZE)
    {
        resetCmd();
        screen << "\nError: Command too long\n";
    }
    else
    {
        cmd[cmdIdx++] = ch;
    }
}

void Shell::processCmd()
{
    if (strlen(cmd) != 0)
    {
        bool found = false;
        for (unsigned int i = 0; i < NUM_COMMANDS; ++i)
        {
            if (strcmp(cmd, COMMAND_NAMES[i]) == 0)
            {
                (this->* COMMANDS[i])();
                found = true;
                break;
            }
        }

        if (!found)
        {
            screen << "Unknown command\n";
        }
    }

    resetCmd();
    prompt();
}

void Shell::resetCmd()
{
    cmdIdx = 0;
}

void Shell::clearCmd()
{
    screen.clear();
}

void Shell::printCmd()
{
    screen << "The print command\n";
}
