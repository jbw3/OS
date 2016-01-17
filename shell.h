#ifndef _SHELL_H
#define _SHELL_H

class Shell
{
public:
    Shell();

    void update();

private:
    static const unsigned int CMD_MAX_SIZE = 64;

    static constexpr unsigned int NUM_COMMANDS = 2;
    static constexpr const char* COMMAND_NAMES[NUM_COMMANDS] =
    {
        "clear",
        "print",
    };

    unsigned int cmdIdx = 0;
    char cmd[CMD_MAX_SIZE + 1]; // add 1 for null char

    void prompt();

    void processChar(char ch);

    void processCmd();

    void resetCmd();

    // ------ Commands ------

    void clearCmd();

    void printCmd();

    typedef void (Shell::* cmdPtr)();

    static constexpr cmdPtr COMMANDS[NUM_COMMANDS] =
    {
        &Shell::clearCmd,
        &Shell::printCmd,
    };
};

#endif // _SHELL_H
