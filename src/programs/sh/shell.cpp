#include <ctype.h>
#include "os.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/wait.h"
#include "unistd.h"

#include "shell.h"

const char ESCAPE = '\x1b';
const char DELETE = '\x7f';

Shell::Commands::iterator::iterator(int builtInIndex, int moduleIndex)
{
    builtInIdx = builtInIndex;
    moduleIdx = moduleIndex;
}

Shell::Commands::iterator Shell::Commands::iterator::operator++()
{
    if (builtInIdx < Shell::NUM_BUILT_IN_COMMANDS)
    {
        ++builtInIdx;
    }
    else if (moduleIdx < getNumModules())
    {
        ++moduleIdx;
    }

    return *this;
}

const char* Shell::Commands::iterator::operator*()
{
    if (builtInIdx < Shell::NUM_BUILT_IN_COMMANDS)
    {
        return Shell::BUILT_IN_COMMANDS[builtInIdx];
    }
    else if (moduleIdx < getNumModules())
    {
        getModuleName(moduleIdx, moduleName);
        return moduleName;
    }
    else
    {
        return nullptr;
    }
}

bool Shell::Commands::iterator::operator==(const Shell::Commands::iterator& other) const
{
    return builtInIdx == other.builtInIdx && moduleIdx == other.moduleIdx;
}

bool Shell::Commands::iterator::operator!=(const Shell::Commands::iterator& other) const
{
    return !(*this == other);
}

Shell::Commands::iterator Shell::Commands::begin()
{
    return iterator();
}

Shell::Commands::iterator Shell::Commands::end()
{
    return iterator(Shell::NUM_BUILT_IN_COMMANDS, getNumModules());
}

const char* Shell::BUILT_IN_COMMANDS[] =
{
    "exit",
    "help",
    "history"
};

const char* Shell::PROMPT = "> ";

Shell::Shell()
{
    cmd[0] = '\0';
    for (int i = 0; i < MAX_HISTORY_SIZE; ++i)
    {
        history[i][0] = '\0';
    }

    // set history size to 1 (the history includes
    // the current command being edited)
    historySize = 1;

    historyIdx = 0;
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

    for (const char* newCmd : commands)
    {
        if (strncmp(cmd, newCmd, origCmdLen) == 0)
        {
            if (bestMatch[0] == '\0')
            {
                strcpy(bestMatch, newCmd);
                strcat(bestMatch, " ");
            }
            else
            {
                size_t j = origCmdLen;
                while (bestMatch[j] != '\0' && bestMatch[j] == newCmd[j])
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

    // reset history index
    historyIdx = 0;

    char escapeSeq[3];
    int escapeSize = 0;
    int cmdSize = 0;
    cmd[0] = '\0';
    char key = getchar();
    while (key != '\r' && key != '\n')
    {
        if (key == ESCAPE || escapeSize > 0)
        {
            escapeSeq[escapeSize++] = key;

            if (escapeSize >= 3)
            {
                if (escapeSeq[1] == 91 && escapeSeq[2] == 65)
                {
                    if (historyIdx < historySize - 1)
                    {
                        if (historyIdx == 0)
                        {
                            strcpy(history[0], cmd);
                        }

                        ++historyIdx;
                        const char* newCmd = history[historyIdx];
                        setCommand(newCmd);
                        cmdSize = strlen(newCmd);
                    }
                }
                else if (escapeSeq[1] == 91 && escapeSeq[2] == 66)
                {
                    if (historyIdx > 0)
                    {
                        --historyIdx;
                        const char* newCmd = history[historyIdx];
                        setCommand(newCmd);
                        cmdSize = strlen(newCmd);
                    }
                }

                // reset size
                escapeSize = 0;
            }
        }
        else if (key == '\b' || key == DELETE)
        {
            if (cmdSize > 0)
            {
                --cmdSize;
                printf("\b \b");
            }
        }
        else if (key == '\t')
        {
            cmdSize = complete();
        }
        else if ( isprint(key) && cmdSize < MAX_CMD_SIZE - 1 )
        {
            cmd[cmdSize++] = key;
            putchar(key);
        }

        cmd[cmdSize] = '\0';

        key = getchar();
    }

    putchar('\n');
}

void Shell::setCommand(const char* newCmd)
{
    char chars[MAX_CMD_SIZE * 3 + 1];

    // overwrite current command in terminal
    size_t cmdLen = strlen(cmd);
    memset(chars, '\b', cmdLen);
    memset(chars + cmdLen, ' ', cmdLen);
    memset(chars + 2 * cmdLen, '\b', cmdLen);
    chars[3 * cmdLen] = '\0';
    printf("%s%s", chars, newCmd);

    strcpy(cmd, newCmd);
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
                // don't add arg if it's empty
                if (argStrIdx > 0 && argStrings[argStrIdx - 1] != '\0')
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
    else if (strcmp(args[0], "history") == 0)
    {
        printHistory();
        found = true;
    }

    return found;
}

void Shell::runCommand()
{
    parseCommand();

    if (args[0] != nullptr)
    {
        // add command to history
        addToHistory();

        if (!runBuiltInCommand())
        {
            const char* name = args[0];

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

        // shift history to make room for new command
        shiftHistory();
    }
}

void Shell::shiftHistory()
{
    if (historySize < MAX_HISTORY_SIZE)
    {
        // shift history
        for (int i = historySize; i > 0; --i)
        {
            strcpy(history[i], history[i - 1]);
        }

        ++historySize;
    }
    else
    {
        // shift history
        for (int i = MAX_HISTORY_SIZE - 1; i > 0; --i)
        {
            strcpy(history[i], history[i - 1]);
        }
    }
}

void Shell::addToHistory()
{
    // copy command to history
    strcpy(history[0], cmd);
}

void Shell::printHelp()
{
    for (const char* cmdName : commands)
    {
        printf(" %s\n", cmdName);
    }
}

void Shell::printHistory()
{
    // Note: Don't print the last item in the history since
    // it won't be accessible to the user.
    int maxIdx = historySize - 1;
    if (historySize >= MAX_HISTORY_SIZE)
    {
        --maxIdx;
    }

    for (int i = maxIdx; i >= 0; --i)
    {
        printf(" %s\n", history[i]);
    }
}
