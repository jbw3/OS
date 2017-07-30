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
#include "pci.h"
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
    mem::PageTable::initKernelPageTablePointer();

    // set up page table page table
    // 1. allocate new page frame which will be the ptpagetable
    uint32_t pageTablePTPhysAddr = PageFrameMgr::get()->allocPageFrame();
    // 2. map this page frame into the next available slot in the current page table
    //    so we can access the entries inside the ptpagetable
    auto pdeIndex = mem::lastUsedKernelPDEIndex();
    mem::PageTable currentPT(pdeIndex);
    screen << "PTR: " << os::Screen::hex << (uint32_t)currentPT.getPointer() << "\n";

    auto pageTablePTAddr = currentPT.mapNextAvailablePageToAddress(pageTablePTPhysAddr);

    uint32_t* pageTablePT = (uint32_t*)(pageTablePTAddr);
    uint16_t ptptIndex = pdeIndex + 1;

    mem::PageTable::initPTPageTable(pageTablePT, ptptIndex);
    // 3. map a PDE to the ptpagetable so that future page tables
    //    can be fully mapped in virtual address space (pageDir[PDE]->PTPT[PTE]->newPageTable)
    // TODO
    auto kPageDir = getKernelPageDirStart();
    kPageDir[ptptIndex] = (PAGE_DIR_ADDRESS & (uint32_t)(pageTablePTPhysAddr)) |
                          (PAGE_DIR_READ_WRITE) |
                          (PAGE_DIR_PRESENT);

    screen << "PTPT page frame addr: 0x" << pageTablePTPhysAddr << "\n";
    screen << "PTPT virt addr: 0x" << (uint32_t)(pageTablePT) << "\n";

    // cls: be careful setting up the PTPT, since some of the PageTable
    // class's functionality depends on the existence of the PTPT.
    // ---------------
    // we can create a PageTable* reference to it as long as we provide a
    // constructor that supplies the pointer to the page table explicitly

    screen.write("Sandbox OSa\n");
    screen.write(*getKernelPageDirStart());
    screen.write("\n");

    Acpi::get();    // force ACPI initialization here
    auto pci = Pci::get();     // force PCI initialization here

    for (int i = 0; i < pci->numDevices(); i++)
    {
        PciDevice* dev = &pci->devices()[i];
        if (dev->header()->classCode == PCI_CLASS_MASS_STRG &&
            dev->header()->subclass == 0x06 &&
            dev->header()->progIF == 0x01)
        {
            screen << "found AHCI device\n";
            dev->printDeviceInfo();
        }
    }

    Shell sh(mbootInfo);

    while (true)
    {
        os::Keyboard::processQueue();

        sh.update();

        // halt CPU until an interrupt occurs
        asm volatile ("hlt");
    }
}
