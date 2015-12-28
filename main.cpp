// main.c

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "screen.h"

struct multiboot;

extern "C"
int kernelMain(struct multiboot* mbootPtr)
{
    initGdt();
    initIdt();
    initIrq();

    // enable interrupts
    asm volatile ("sti");

    screen.init();
    screen.setBackgroundColor(os::Screen::EColor::eBlack);
    screen.setForegroundColor(os::Screen::EColor::eLightGreen);
    screen.clear();

    screen.write("Sandbox OS\n");

    while (true);

    return 0;
}
