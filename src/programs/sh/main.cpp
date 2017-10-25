#include "shell.h"

int main(int argc, const char* argv[])
{
    Shell shell;
    return shell.execute(argc, argv);
}
