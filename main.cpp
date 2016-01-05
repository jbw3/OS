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
 * @brief Debugging function to print Multiboot info
 * @param mbootInfo pointer to Multiboot info struct
 */
void printMultibootInfo(const multiboot_info* mbootInfo);

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

    printMultibootInfo(mbootInfo);

    while (true)
    {
        os::Keyboard::processQueue();

        // halt CPU until an interrupt occurs
        asm volatile ("hlt");
    }
}

void printMultibootInfo(const multiboot_info* mbootInfo)
{
    screen << "Multiboot info:\n";

    // print flags
    screen << os::Screen::bin
           << "Flags: " << mbootInfo->flags << '\n'
           << os::Screen::dec;

    uint32_t bit = 1;
    for (int i = 0; i < 32; ++i)
    {
        bool isSet = (mbootInfo->flags & bit);
        if (isSet)
        {
            screen << "Bit " << i << ": ";
            switch (bit)
            {
            case MULTIBOOT_INFO_MEMORY:
                screen << mbootInfo->mem_lower << " KB, " << mbootInfo->mem_upper << " KB\n";
                break;

            case MULTIBOOT_INFO_BOOTDEV:
                screen << os::Screen::hex
                       << mbootInfo->boot_device << '\n'
                       << os::Screen::dec;
                break;

            case MULTIBOOT_INFO_CMDLINE:
                screen << reinterpret_cast<const char*>(mbootInfo->cmdline) << '\n';
                break;

            case MULTIBOOT_INFO_MODS:
                screen << mbootInfo->mods_count << " boot module" << (mbootInfo->mods_count == 1 ? "" : "s") << " were loaded\n";
                break;

            case MULTIBOOT_INFO_MEM_MAP:
                screen << mbootInfo->mmap_length << '\n';
                break;

            case MULTIBOOT_INFO_BOOT_LOADER_NAME:
                screen << reinterpret_cast<const char*>(mbootInfo->boot_loader_name) << '\n';
                break;

            default:
                screen << '\n';
                break;
            }
        }

        bit <<= 1;
    }
}
