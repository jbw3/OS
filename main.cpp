#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "keyboard.h"
#include "screen.h"
#include "timer.h"

struct multiboot;

/**
 * @brief 32-bit x86 kernel main
 */
extern "C"
void kernelMain(struct multiboot* mbootPtr)
{
    initGdt();
    initIdt();
    initIrq();

    os::Timer::init(18);

    screen.init();
    screen.setBackgroundColor(os::Screen::EColor::eBlack);
    screen.setForegroundColor(os::Screen::EColor::eLightGreen);
    screen.clear();

    os::Keyboard::init();

    // enable interrupts
    asm volatile ("sti");

    screen.write("Sandbox OS\n");

    while (true)
    {
        os::Keyboard::processQueue();

        // halt CPU until an interrupt occurs
        asm volatile ("hlt");
    }
}
