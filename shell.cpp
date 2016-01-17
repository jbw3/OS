#include "debug.h"
#include "paging.h"
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
        screen << ch;
        cmd[cmdIdx] = '\0';
        processCmd();
    }
    else if (ch == '\b')
    {
        if (cmdIdx > 0)
        {
            // clear the previous char
            screen << "\b \b";
            --cmdIdx;
        }
    }
    else if (cmdIdx == CMD_MAX_SIZE)
    {
        screen << ch;
        screen << "\nError: Command too long\n";
        resetCmd();
        prompt();
    }
    else
    {
        screen << ch;
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
    else if (strcmp(arg, "paging") == 0)
    {
        char* state = strtok(nullptr, " ");
        if (state == nullptr)
        {
            screen << "No state given\n";
        }
        else if (strcmp(state, "off") == 0)
        {
            disablePaging();
        }
        else if (strcmp(state, "on") == 0)
        {
            enablePaging();
        }
        else
        {
            screen << "Invalid state\n";
        }
    }
    else
    {
        screen << "Unexpected argument\n";
    }
}

void Shell::showCmd()
{
    char* arg1 = strtok(nullptr, " ");
    if (arg1 == nullptr)
    {
        screen << "Not enough arguments\n";
    }
    else if (strcmp(arg1, "mboot") == 0)
    {
        char* arg2 = strtok(nullptr, " ");
        if (arg2 == nullptr)
        {
            screen << "Not enough arguments\n";
        }
        else if (strcmp(arg2, "info") == 0)
        {
            printMultibootInfo(mbootInfo);
        }
        else if (strcmp(arg2, "mem") == 0)
        {
            printMemMap(mbootInfo->mmap_addr, mbootInfo->mmap_length);
        }
        else
        {
            screen << "Unexpected argument\n";
        }
    }
    else if (strcmp(arg1, "smile") == 0 || strcmp(arg1, "smiley") == 0)
    {
        for (int i = 0; i < 20; ++i)
        {
            screen << "\1 \2 ";
        }
    }
    else
    {
        screen << "Unexpected argument\n";
    }
}
