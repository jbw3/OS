#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "multiboot.h"

#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "keyboard.h"
#include "screen.h"
#include "timer.h"

/**
 * @brief 32-bit x86 kernel main
 */
extern "C"
void kernelMain(const uint32_t MULTIBOOT_MAGIC_NUM, const multiboot_info* mbootInfo)
{
    screen.init();
    screen.setBackgroundColor(os::Screen::EColor::eBlack);
    screen.setForegroundColor(os::Screen::EColor::eLightGreen);
    screen.clear();

    // ensure we were booted by a Multiboot-compliant boot loader
    if (MULTIBOOT_MAGIC_NUM != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        screen << "Invalid Multiboot magic number: "
               << os::Screen::hex << MULTIBOOT_MAGIC_NUM
               << '\n';
        return;
    }

    initGdt();
    initIdt();
    initIrq();

    os::Timer::init(18);

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
