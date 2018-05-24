#include <stdio.h>

int main()
{
    // send escape sequences to clear the screen and
    // reset the cursor to the top left of the screen
    printf("\x1B[2J\x1B[H");

    return 0;
}
