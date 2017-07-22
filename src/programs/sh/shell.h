#ifndef SHELL_H_
#define SHELL_H_

class Shell
{
public:
    Shell();

    void mainloop();

private:
    static constexpr int MAX_CMD_SIZE = 128;
    static const char* PROMPT;
    char cmd[MAX_CMD_SIZE];

    void getCommand();

    void runCommand();
};

#endif // SHELL_H_
