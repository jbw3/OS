// main.c

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "screen.h"

struct multiboot;

extern "C"
int kernelMain(struct multiboot* mbootPtr)
{
    os::Screen screen;

    screen.setBackgroundColor(os::Screen::EColor::eBlack);
    screen.setForegroundColor(os::Screen::EColor::eLightGreen);
    screen.clear();

    screen.write("SandboxOS\n");

    while (true);

    return 0;
}
