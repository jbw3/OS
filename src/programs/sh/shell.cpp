#include "os.h"
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
    done = false;
}

int Shell::execute(int argc, const char* argv[])
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "-c") == 0)
        {
            if (argc == 2)
            {
                printf("-c requires a command string as an argument\n");
                return 1;
            }
            else
            {
                strcpy(cmd, argv[2]);
                runCommand();
            }
        }
        else
        {
            printf("Unknown argument '%s'\n", argv[1]);
            return 1;
        }
    }
    else
    {
        interactiveLoop();
    }

    return 0;
}

void Shell::interactiveLoop()
{
    while (!done)
    {
        getCommand();
        runCommand();
    }
}

size_t Shell::complete()
{
    char bestMatch[MAX_CMD_SIZE] = "";
    size_t origCmdLen = strlen(cmd);

    int numModules = getNumModules();
    for (int i = 0; i < numModules; ++i)
    {
        char moduleName[MAX_CMD_SIZE];
        getModuleName(i, moduleName);

        if (strncmp(cmd, moduleName, origCmdLen) == 0)
        {
            if (bestMatch[0] == '\0')
            {
                strcpy(bestMatch, moduleName);
                strcat(bestMatch, " ");
            }
            else
            {
                size_t j = origCmdLen;
                while (bestMatch[j] != '\0' && bestMatch[j] == moduleName[j])
                {
                    ++j;
                }
                bestMatch[j] = '\0';
            }
        }
    }

    if (bestMatch[0] == '\0')
    {
        return origCmdLen;
    }

    strcpy(cmd, bestMatch);
    size_t newCmdLen = strlen(cmd);
    for (size_t i = origCmdLen; i < newCmdLen; ++i)
    {
        putchar(bestMatch[i]);
    }

    return newCmdLen;
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
        else if (ch == '\t')
        {
            cmd[cmdSize] = '\0';
            cmdSize = complete();
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
    int numArgs = 0;
    bool inQuotes = false;
    char quoteChar = '\0';
    size_t cmdIdx = 0;
    size_t argStrIdx = 0;
    char ch = '\0';
    args[numArgs] = argStrings;

    bool done = false;
    while (!done && numArgs < MAX_ARGS_SIZE - 1)
    {
        ch = cmd[cmdIdx];

        if (inQuotes)
        {
            if (ch == quoteChar)
            {
                argStrings[argStrIdx++] = '\0';
                args[++numArgs] = &argStrings[argStrIdx];
                inQuotes = false;
            }
            else if (ch == '\0')
            {
                argStrings[argStrIdx++] = '\0';
                args[++numArgs] = &argStrings[argStrIdx];
            }
            else
            {
                argStrings[argStrIdx++] = ch;
            }
        }
        else
        {
            if (ch == '\'' || ch == '"')
            {
                inQuotes = true;
                quoteChar = ch;
            }
            else if (ch == ' ' || ch == '\t' || ch == '\0')
            {
                // don't add arg if the whitespace charactor was at the start
                // of the string or after another whitespace character
                if (cmdIdx != 0 && cmd[cmdIdx - 1] != ' ' && cmd[cmdIdx - 1] != '\t')
                {
                    argStrings[argStrIdx++] = '\0';
                    args[++numArgs] = &argStrings[argStrIdx];
                }
            }
            else
            {
                argStrings[argStrIdx++] = ch;
            }
        }

        done = (ch == '\0');
        ++cmdIdx;
    }

    args[numArgs] = nullptr;
}

bool Shell::runBuiltInCommand()
{
    bool found = false;

    if (strcmp(args[0], "exit") == 0)
    {
        done = true;
        found = true;
    }
    else if (strcmp(args[0], "help") == 0)
    {
        printHelp();
        found = true;
    }

    return found;
}

void Shell::runCommand()
{
    parseCommand();

    if (!runBuiltInCommand())
    {
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
}

void Shell::printHelp()
{
    int numModules = getNumModules();
    for (int i = 0; i < numModules; ++i)
    {
        char name[128];
        getModuleName(i, name);

        printf("%s\n", name);
    }
}
