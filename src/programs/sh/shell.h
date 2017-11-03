#ifndef SHELL_H_
#define SHELL_H_

#include <stddef.h>

class Shell
{
public:
    Shell();

    int execute(int argc, const char* argv[]);

private:
    static constexpr int MAX_CMD_SIZE = 100;
    static constexpr int MAX_ARGS_SIZE = 32;
    static constexpr int MAX_TOTAL_ARGS_SIZE = MAX_CMD_SIZE;
    static constexpr int NUM_BUILT_IN_COMMANDS = 3;
    static const char* BUILT_IN_COMMANDS[NUM_BUILT_IN_COMMANDS];
    static constexpr int MAX_HISTORY_SIZE = 10;
    static const char* PROMPT;

    class Commands
    {
    public:
        class iterator
        {
        public:
            iterator(int builtInIndex = 0, int moduleIndex = 0);

            iterator operator++();

            const char* operator*();

            bool operator==(const iterator& other) const;

            bool operator!=(const iterator& other) const;
        private:
            int builtInIdx;
            int moduleIdx;
            char moduleName[Shell::MAX_CMD_SIZE];
        };

        iterator begin();

        iterator end();
    } commands;

    char cmd[MAX_CMD_SIZE];
    char* args[MAX_ARGS_SIZE];
    char argStrings[MAX_TOTAL_ARGS_SIZE];
    char history[MAX_HISTORY_SIZE][MAX_CMD_SIZE];
    int historySize;
    int historyIdx;
    bool done;

    void interactiveLoop();

    size_t complete();

    void getCommand();

    void setCommand(const char* newCmd);

    void parseCommand();

    bool runBuiltInCommand();

    void runCommand();

    void shiftHistory();

    void addToHistory();

    void printHelp();

    void printHistory();
};

#endif // SHELL_H_
