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

void Shell::runCommand()
{
    char* name = strtok(cmd, " ");
    if (name != nullptr)
    {
        pid_t pid = fork();
        if (pid < 0)
        {
            printf("Error: Could not run command.\n");
        }
        else if (pid == 0)
        {
            // copy args
            const int MAX_ARGS = 64;
            char* argv[MAX_ARGS];
            argv[0] = name;

            int idx = 1;
            char* arg = nullptr;
            do
            {
                arg = strtok(nullptr, " ");
                argv[idx++] = arg;

                if (idx >= MAX_ARGS - 1)
                {
                    argv[MAX_ARGS - 1] = nullptr;
                    break;
                }
            } while (arg != nullptr);

            // execute command
            execv(name, argv);

            printf("Could not find command '%s'.\n", name);
            exit(-1);
        }

        // wait for child process to finish
        wait(nullptr);
    }
}
