#include "debug.h"
#include "paging.h"
#include "screen.h"
#include "shell.h"
#include "stdlib.h"
#include "string.h"
#include "timer.h"

class Command
{
public:
    virtual const char* getName() = 0;

    virtual const char* getHelp() = 0;

    virtual void execute() = 0;
};

class ClearCommand : public Command
{
public:
    const char* getName() override
    {
        return "clear";
    }

    const char* getHelp() override
    {
        return "Clears the screen\n";
    }

    void execute() override
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
} clearCmd;

class ReadCommand : public Command
{
public:
    const char* getName() override
    {
        return "read";
    }

    const char* getHelp() override
    {
        return
R"(Reads a byte from memory
)";
    }

    void execute() override
    {
        char* arg = strtok(nullptr, " ");
        if (arg == nullptr)
        {
            screen << "Not enough arguments\n";
        }
        else if (strtok(nullptr, " ") != nullptr)
        {
            screen << "Too many arguments\n";
        }
        else
        {
            char* str = nullptr;
            long addr = strtol(arg, &str, 16);
            if (str == nullptr || *str != '\0')
            {
                screen << "Argument is not a valid hexadecimal number\n";
            }
            else
            {
                uint8_t* ptr = reinterpret_cast<uint8_t*>(addr);
                uint8_t value = *ptr;
                screen << os::Screen::hex
                       << os::Screen::setfill('0')
                       << os::Screen::setw(2)
                       << value
                       << os::Screen::setfill(' ')
                       << os::Screen::dec
                       << '\n';
            }
        }
    }
} readCmd;

class SetCommand : public Command
{
public:
    const char* getName() override
    {
        return "set";
    }

    const char* getHelp() override
    {
        return
R"(Changes config options:

set bg <0-15>
    Set background color

set fg <0-15>
    Set foreground color

set paging <on|off>
    Enable/disable paging
)";
    }

    void execute() override
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
} setCmd;

class ShowCommand : public Command
{
public:
    ShowCommand() :
        mbootInfo(nullptr)
    {
    }

    void setMbootInfo(const multiboot_info* mbootInfoPtr)
    {
        mbootInfo = mbootInfoPtr;
    }

    const char* getName() override
    {
        return "show";
    }

    const char* getHelp() override
    {
        return
R"(Displays information:

show drives [all|master|slave]
    List drives

show mboot info
    Display general multiboot information
show mboot mod
    Display multiboot module information
show mboot mem
    Display multiboot memory map
show mboot drives
    Display multiboot drive information

show pagedir <start index> [end index]
    Display page directory entries, index range: 0-1023
show pagetab <page dir index> <start index> [end index]
    Display page table entries, index range: 0-1023

show ticks
    Display timer ticks since system boot
)";
    }

    void execute() override
    {
        char* arg1 = strtok(nullptr, " ");
        if (arg1 == nullptr)
        {
            screen << "Not enough arguments\n";
        }
        else if (strcmp(arg1, "drives") == 0)
        {
            char* arg2 = strtok(nullptr, " ");
            char* arg3 = strtok(nullptr, " ");
            if (arg3 != nullptr)
            {
                screen << "Too many arguments\n";
            }
            else if (arg2 == nullptr || strcmp(arg2, "all") == 0)
            {
                printDrives(true, true);
            }
            else if (strcmp(arg2, "master") == 0)
            {
                printDrives(true, false);
            }
            else if (strcmp(arg2, "slave") == 0)
            {
                printDrives(false, true);
            }
            else
            {
                screen << "Unknown argument: \"" << arg2 << "\"\n";
            }
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
                printMultibootMemMap(mbootInfo->mmap_addr, mbootInfo->mmap_length);
            }
            else if (strcmp(arg2, "mod") == 0)
            {
                printMultibootModules(mbootInfo->mods_addr, mbootInfo->mods_count);
            }
            else if (strcmp(arg2, "drives") == 0)
            {
                printMultibootDrives(mbootInfo->drives_addr, mbootInfo->drives_length);
            }
            else
            {
                screen << "Unexpected argument\n";
            }
        }
        else if (strcmp(arg1, "pagedir") == 0)
        {
            char* arg2 = strtok(nullptr, " ");
            if (arg2 == nullptr)
            {
                screen << "Not enough arguments\n";
            }
            else
            {
                int startIdx = atoi(arg2);
                int endIdx = startIdx;

                char* arg3 = strtok(nullptr, " ");
                if (arg3 != nullptr)
                {
                    endIdx = atoi(arg3);
                }

                if (startIdx < 0 || startIdx >= 1024)
                {
                    screen << "Invalid start index\n";
                }
                else if (endIdx < 0 || endIdx >= 1024)
                {
                    screen << "Invalid end index\n";
                }
                else
                {
                    printPageDir(startIdx, endIdx);
                }
            }
        }
        else if (strcmp(arg1, "pagetab") == 0)
        {
            char* arg2 = strtok(nullptr, " ");
            char* arg3 = strtok(nullptr, " ");
            if (arg2 == nullptr || arg3 == nullptr)
            {
                screen << "Not enough arguments\n";
            }
            else
            {
                int pageDirIdx = atoi(arg2);
                int startIdx = atoi(arg3);
                int endIdx = startIdx;

                char* arg4 = strtok(nullptr, " ");
                if (arg4 != nullptr)
                {
                    endIdx = atoi(arg4);
                }

                if (startIdx < 0 || startIdx >= 1024)
                {
                    screen << "Invalid start index\n";
                }
                else if (endIdx < 0 || endIdx >= 1024)
                {
                    screen << "Invalid end index\n";
                }
                else
                {
                    printPageTable(pageDirIdx, startIdx, endIdx);
                }
            }
        }
        else if (strcmp(arg1, "smile") == 0 || strcmp(arg1, "smiley") == 0)
        {
            for (int i = 0; i < 20; ++i)
            {
                screen << "\1 \2 ";
            }
        }
        else if (strcmp(arg1, "ticks") == 0)
        {
            char* arg2 = strtok(nullptr, " ");
            if (arg2 != nullptr)
            {
                screen << "Unexpected argument \"" << arg2 << "\"\n";
            }
            else
            {
                screen << os::Timer::getTicks() << '\n';
            }
        }
        else
        {
            screen << "Unexpected argument\n";
        }
    }

private:
    const multiboot_info* mbootInfo;
} showCmd;

class WriteCommand : public Command
{
public:
    const char* getName() override
    {
        return "write";
    }

    const char* getHelp() override
    {
        return
R"(Writes a byte to memory
)";
    }

    void execute() override
    {
        char* addrArg = strtok(nullptr, " ");
        char* valArg = strtok(nullptr, " ");
        if (addrArg == nullptr || valArg == nullptr)
        {
            screen << "Not enough arguments\n";
        }
        else if (strtok(nullptr, " ") != nullptr)
        {
            screen << "Too many arguments\n";
        }
        else
        {
            bool ok = true;

            char* str = nullptr;
            long addr = strtol(addrArg, &str, 16);
            if (str == nullptr || *str != '\0')
            {
                screen << "Address is not a valid hexadecimal number\n";
                ok = false;
            }

            str = nullptr;
            uint8_t val = strtol(valArg, &str, 16);
            if (str == nullptr || *str != '\0')
            {
                screen << "Value is not a valid number\n";
                ok = false;
            }

            if (ok)
            {
                uint8_t* ptr = reinterpret_cast<uint8_t*>(addr);
                *ptr = val;
            }
        }
    }
} writeCmd;

Command* Shell::COMMANDS[NUM_COMMANDS] =
{
    &clearCmd,
    &readCmd,
    &setCmd,
    &showCmd,
    &writeCmd,
};

Shell::Shell(const multiboot_info* mbootInfoPtr) :
    mbootInfo(mbootInfoPtr)
{
    showCmd.setMbootInfo(mbootInfo);

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
        if (strcmp(token, "help") == 0)
        {
            found = true;
            displayHelp();
        }

        // look for command object
        if (!found)
        {
            for (unsigned int i = 0; i < NUM_COMMANDS; ++i)
            {
                Command* cmd = COMMANDS[i];
                if (strcmp(token, cmd->getName()) == 0)
                {
                    cmd->execute();
                    found = true;
                    break;
                }
            }

        }

        // look for program
        if (!found)
        {
            uint32_t progAddr = 0;
            found = findProgram(token, progAddr);
            if (found)
            {
                runProgram(progAddr);
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

bool Shell::findProgram(const char* name, uint32_t& progAddr)
{
    bool found = false;
    uint32_t addr = mbootInfo->mods_addr;
    uint32_t count = mbootInfo->mods_count;

    for (uint32_t i = 0; i < count; ++i)
    {
        // get module info struct
        const multiboot_mod_list* module = reinterpret_cast<const multiboot_mod_list*>(addr);

        // check if name matches
        const char* modName = reinterpret_cast<const char*>(module->cmdline);
        if (strcmp(name, modName) == 0)
        {
            progAddr = module->mod_start;
            found = true;
            break;
        }

        addr += sizeof(multiboot_mod_list);
    }

    return found;
}

void Shell::runProgram(uint32_t addr)
{
    programPtr program = reinterpret_cast<programPtr>(addr);
    int rc = program();
    screen << "Return code: " << rc << '\n';
}

void Shell::displayHelp()
{
    char* arg = strtok(nullptr, " ");

    // if no argument was given, list the commands
    if (arg == nullptr)
    {
        screen << "Commands:\n";
        for (unsigned int i = 0; i < NUM_COMMANDS; ++i)
        {
            screen << COMMANDS[i]->getName() << '\n';
        }
    }
    else if (strtok(nullptr, " ") != nullptr)
    {
        screen << "Only expected one argument\n";
    }
    else if (strcmp(arg, "help") == 0)
    {
        screen << "Displays help for commands\n";
    }
    else
    {
        bool found = false;
        for (unsigned int i = 0; i < NUM_COMMANDS; ++i)
        {
            Command* cmd = COMMANDS[i];
            if (strcmp(arg, cmd->getName()) == 0)
            {
                screen << cmd->getHelp();
                found = true;
                break;
            }
        }

        if (!found)
        {
            screen << '"' << arg << "\" is not a command\n";
        }
    }
}

void Shell::resetCmd()
{
    cmdIdx = 0;
}
