#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/wait.h"
#include "unistd.h"

#include "shell.h"

const char* Shell::PROMPT = "> ";

Shell::Shell()
{
    cmd[0] = '\0';
}

void Shell::mainloop()
{
    while (true)
    {
        getCommand();

        runCommand();
    }
}

void Shell::getCommand()
{
    printf("%s", Shell::PROMPT);

    int cmdSize = 0;
    char ch = getchar();
    while (ch != '\n')
    {
        if (ch == '\b')
        {
            if (cmdSize > 0)
            {
                --cmdSize;
                printf("\b \b");
            }
        }
        else if (cmdSize < MAX_CMD_SIZE - 1)
        {
            cmd[cmdSize++] = ch;
            putchar(ch);
        }

        ch = getchar();
    }

    putchar('\n');
    cmd[cmdSize] = '\0';
}

void Shell::parseCommand()
{
    char* argStrPtr = argStrings;
    int numArgs = 0;
    char* arg = strtok(cmd, " ");
    while (arg != nullptr && numArgs < MAX_ARGS_SIZE - 1)
    {
        strcpy(argStrPtr, arg);
        args[numArgs++] = argStrPtr;

        argStrPtr += strlen(arg) + 1;
        arg = strtok(nullptr, " ");
    }

    args[numArgs] = nullptr;
}

void Shell::runCommand()
{
    parseCommand();

    const char* name = args[0];

    if (name != nullptr)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            printf("Error: Could not run command.\n");
        }
        else if (pid == 0)
        {
            // execute command
            execv(name, args);

            printf("Could not find command '%s'.\n", name);
            exit(-1);
        }

        // wait for child process to finish
        wait(nullptr);
    }
}
