#include "debug.h"
#include "screen.h"
#include "shell.h"
#include "stdlib.h"
#include "string.h"

const char* Shell::COMMAND_NAMES[NUM_COMMANDS] =
{
    "clear",
    "set",
    "show",
};

const cmdPtr Shell::COMMANDS[NUM_COMMANDS] =
{
    &Shell::clearCmd,
    &Shell::setCmd,
    &Shell::showCmd,
};

Shell::Shell(const multiboot_info* mbootInfoPtr) :
    mbootInfo(mbootInfoPtr)
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
        screen << "\nError: Command too long\n";
        resetCmd();
        prompt();
    }
    else
    {
        cmd[cmdIdx++] = ch;
    }
}

void Shell::processCmd()
{
    char* token = strtok(cmd, " ");
    if (token != nullptr)
    {
        bool found = false;
        for (unsigned int i = 0; i < NUM_COMMANDS; ++i)
        {
            if (strcmp(token, COMMAND_NAMES[i]) == 0)
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
    if (strtok(nullptr, " ") != nullptr)
    {
        screen << "Unexpected arguments\n";
    }
    else
    {
        screen.clear();
    }
}

void Shell::setCmd()
{
    char* arg = strtok(nullptr, " ");
    if (arg == nullptr)
    {
        screen << "Not enough arguments\n";
    }
    else if (strcmp(arg, "bg") == 0)
    {
        char* colorStr = strtok(nullptr, " ");
        if (colorStr == nullptr)
        {
            screen << "No color given\n";
        }
        else
        {
            int color = atoi(colorStr);
            if (color < 0 || color > 15)
            {
                screen << "Invalid color\n";
            }
            else
            {
                screen.setBackgroundColor(static_cast<os::Screen::EColor>(color));
            }
        }
    }
    else if (strcmp(arg, "fg") == 0)
    {
        char* colorStr = strtok(nullptr, " ");
        if (colorStr == nullptr)
        {
            screen << "No color given\n";
        }
        else
        {
            int color = atoi(colorStr);
            if (color < 0 || color > 15)
            {
                screen << "Invalid color\n";
            }
            else
            {
                screen.setForegroundColor(static_cast<os::Screen::EColor>(color));
            }
        }
    }
    else
    {
        screen << "Unexpected arguments\n";
    }
}

void Shell::showCmd()
{
    char* arg = strtok(nullptr, " ");
    if (arg == nullptr)
    {
        screen << "Not enough arguments\n";
    }
    else if (strcmp(arg, "mboot") == 0)
    {
        printMultibootInfo(mbootInfo);
    }
    else if (strcmp(arg, "smile") == 0 || strcmp(arg, "smiley") == 0)
    {
        for (int i = 0; i < 20; ++i)
        {
            screen << "\1 \2 ";
        }
    }
    else
    {
        screen << "Unexpected arguments\n";
    }
}
