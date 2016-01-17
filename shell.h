#ifndef _SHELL_H
#define _SHELL_H

struct multiboot_info;

class Shell;

typedef void (Shell::* cmdPtr)();

class Shell
{
public:
    Shell(const multiboot_info* mbootInfoPtr);

    void update();

private:
    static const unsigned int CMD_MAX_SIZE = 64;

    static constexpr unsigned int NUM_COMMANDS = 3;
    static const char* COMMAND_NAMES[NUM_COMMANDS];
    static const cmdPtr COMMANDS[NUM_COMMANDS];

    unsigned int cmdIdx = 0;
    char cmd[CMD_MAX_SIZE + 1]; // add 1 for null char

    const multiboot_info* mbootInfo;

    void prompt();

    void processChar(char ch);

    void processCmd();

    void resetCmd();

    // ------ Commands ------

    void clearCmd();

    void setCmd();

    void showCmd();
};

#endif // _SHELL_H
