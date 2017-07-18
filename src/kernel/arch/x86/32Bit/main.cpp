#include <stdint.h>

#include "multiboot.h"

#include "acpi.h"
#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "keyboard.h"
#include "pageframemgr.h"
#include "pagetable.h"
#include "paging.h"
#include "screen.h"
#include "shell.h"
#include "system.h"
#include "timer.h"
#include "vmem.h"

/**
 * @brief 32-bit x86 kernel main
 */
extern "C"
void kernelMain(const uint32_t MULTIBOOT_MAGIC_NUM, const multiboot_info* mbootInfo)
{
    initGdt();
    initIdt();
    initIrq();

    configPaging();

    os::Timer::init(20);

    os::Keyboard::init();

    // enable interrupts
    asm volatile ("sti");

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

    PageFrameMgr::init(mbootInfo);

    // set up page table page table
    uint32_t pageTablePTPhysAddr = PageFrameMgr::get()->allocPageFrame();
    auto kPageDir = getKernelPageDirStart();
    auto lastPDEIdx = mem::lastUsedKernelPDEIndex();
    mem::PageTable currentPT(kPageDir, lastPDEIdx);
    auto pageTablePTAddr = currentPT.mapNextAvailablePageToAddress(pageTablePTPhysAddr);
    uint32_t* pageTablePT = (uint32_t*)(pageTablePTAddr);

    // cls: be careful setting up the PTPT, since some of the PageTable
    // class's functionality depends on the existence of the PTPT.
    // ---------------
    // we can create a PageTable* reference to it as long as we provide a
    // constructor that supplies the pointer to the page table explicitly

    screen.write("Sandbox OSa\n");
    screen.write(*getKernelPageDirStart());
    screen.write("\n");

    os::Acpi acpi;

    Shell sh(mbootInfo);

    while (true)
    {
        os::Keyboard::processQueue();

        sh.update();

        // halt CPU until an interrupt occurs
        asm volatile ("hlt");
    }
}
