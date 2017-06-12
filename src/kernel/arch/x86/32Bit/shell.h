#ifndef _SHELL_H
#define _SHELL_H

#include <stdint.h>

struct multiboot_info;

class Command;
class ProcessMgr;

class Shell
{
public:
    Shell(const multiboot_info* mbootInfoPtr, ProcessMgr& processMgr);

    void update();

private:
    static const unsigned int CMD_MAX_SIZE = 64;

    static constexpr unsigned int NUM_COMMANDS = 5;
    static Command* COMMANDS[NUM_COMMANDS];

    const multiboot_info* mbootInfo;
    ProcessMgr& processMgr;

    typedef int (*programPtr)();

    unsigned int cmdIdx = 0;
    char cmd[CMD_MAX_SIZE + 1]; // add 1 for null char

    void prompt();

    void processChar(char ch);

    void processCmd();

    bool findProgram(const char* name, const multiboot_mod_list*& module);

    void runProgram(const multiboot_mod_list* module);

    void displayHelp();

    void resetCmd();
};

#endif // _SHELL_H
