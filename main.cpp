// main.c

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gdt.h"
#include "idt.h"
#include "screen.h"

struct multiboot;

extern "C"
int kernelMain(struct multiboot* mbootPtr)
{
    initGdt();
    initIdt();

    os::Screen screen;

    screen.setBackgroundColor(os::Screen::EColor::eBlack);
    screen.setForegroundColor(os::Screen::EColor::eLightGreen);
    screen.clear();

    for (int i = 0; i < 13; ++i)
    {
        screen.write('\n');
    }
    for (int i = 0; i < 35; ++i)
    {
        screen.write(' ');
    }
    screen.write("Sandbox OS");

    while (true);

    return 0;
}
