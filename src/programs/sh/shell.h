#ifndef SHELL_H_
#define SHELL_H_

class Shell
{
public:
    Shell();

    int execute(int argc, const char* argv[]);

private:
    static constexpr int MAX_CMD_SIZE = 128;
    static constexpr int MAX_ARGS_SIZE = 32;
    static constexpr int MAX_TOTAL_ARGS_SIZE = MAX_CMD_SIZE;
    static const char* PROMPT;
    char cmd[MAX_CMD_SIZE];
    char* args[MAX_ARGS_SIZE];
    char argStrings[MAX_TOTAL_ARGS_SIZE];
    bool done;

    void interactiveLoop();

    void getCommand();

    void parseCommand();

    bool runBuiltInCommand();

    void runCommand();

    void printHelp();
};

#endif // SHELL_H_
