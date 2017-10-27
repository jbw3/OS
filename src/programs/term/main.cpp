#include <os.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char HELP[] =
R"(terminal settings:
  term fg <color>
  term bg <color>
)";

int main(int argc, const char* argv[])
{
    int rc = 0;

    if (argc < 2)
    {
        printf("%s", HELP);
        rc = 0;
    }
    else if (strcmp(argv[1], "bg") == 0)
    {
        if (argc < 3)
        {
            printf("Expected a color.\n");
            rc = 1;
        }
        else
        {
            int color = atoi(argv[2]);
            setTerminalBackground(static_cast<EColor>(color));
            rc = 0;
        }
    }
    else if (strcmp(argv[1], "fg") == 0)
    {
        if (argc < 3)
        {
            printf("Expected a color.\n");
            rc = 1;
        }
        else
        {
            int color = atoi(argv[2]);
            setTerminalForeground(static_cast<EColor>(color));
            rc = 0;
        }
    }
    else
    {
        printf("Unknown command: %s\n", argv[1]);
        rc = 1;
    }

    return rc;
}
