#include "multiboot.h"
#include "paging.h"
#include "screen.h"
#include "system.h"
#include "utils.h"

extern "C"
void pageFault(const registers* regs)
{
    os::Screen::EColor bgColor = screen.getBackgroundColor();
    os::Screen::EColor fgColor = screen.getForegroundColor();

    screen << '\n';

    screen.setBackgroundColor(os::Screen::EColor::eRed);
    screen.setForegroundColor(os::Screen::EColor::eWhite);

    screen << "Page fault!\n"
           << "Error code: " << regs->errCode << '\n';

    if ( (regs->errCode & PAGE_ERROR_USER) != 0 )
    {
        screen << "A user process ";
    }
    else
    {
        screen << "A supervisory process ";
    }

    if ( (regs->errCode & PAGE_ERROR_WRITE) != 0 )
    {
        screen << "tried to write to ";
    }
    else
    {
        screen << "tried to read from ";
    }

    if ( (regs->errCode & PAGE_ERROR_PRESENT) != 0 )
    {
        screen << "a page and caused a protection fault.\n";
    }
    else
    {
        screen << "a non-present page.\n";
    }

    screen << "Address: "
           << os::Screen::setw(8)
           << os::Screen::setfill('0')
           << os::Screen::hex
           << getRegCR2()
           << os::Screen::setfill(' ')
           << os::Screen::dec
           << '\n';

    screen.setBackgroundColor(bgColor);
    screen.setForegroundColor(fgColor);

    PANIC("Page fault!");
}

void configPaging()
{
    // register page fault handler
    registerIsrHandler(ISR_PAGE_FAULT, pageFault);
}

void mapPageTable(uint32_t* pageDir, uint32_t pageTableAddr, int pageDirIdx, bool user)
{
    if (pageDirIdx < 0 || pageDirIdx >= 1024)
    {
        PANIC("Invalid page directory index.");
    }

    // add the page table to the page directory
    uint32_t pageDirEntry = 0;
    pageDirEntry |= pageTableAddr & PAGE_DIR_ADDRESS;       // add new address
    pageDirEntry |= PAGE_DIR_READ_WRITE | PAGE_DIR_PRESENT; // set read/write and present bits
    if (user)
    {
        pageDirEntry |= PAGE_DIR_USER; // set user privilege
    }
    pageDir[pageDirIdx] = pageDirEntry;
}

void mapPage(uint32_t* pageTable, uint32_t virtualAddr, uint32_t physicalAddr, bool user)
{
    // calculate the page table index
    int pageTableIdx = (virtualAddr >> 12) & PAGE_TABLE_INDEX_MASK;

    // get the page entry from the page table
    uint32_t pageTableEntry = pageTable[pageTableIdx];

    // add the page address to the page table
    pageTableEntry &= ~PAGE_TABLE_ADDRESS;                        // clear previous address
    pageTableEntry |= physicalAddr & PAGE_TABLE_ADDRESS;          // add new address
    pageTableEntry |= PAGE_TABLE_READ_WRITE | PAGE_TABLE_PRESENT; // set read/write and present bits
    if (user)
    {
        pageTableEntry |= PAGE_TABLE_USER; // set user privilege
    }
    pageTable[pageTableIdx] = pageTableEntry;
}

bool mapPage(int pageDirIdx, uint32_t* pageTable, uint32_t& virtualAddr, uint32_t physicalAddr, bool user)
{
    for (int idx = 0; idx < PAGE_TABLE_NUM_ENTRIES; ++idx)
    {
        uint32_t entry = pageTable[idx];
        if ( (entry & PAGE_TABLE_PRESENT) == 0 )
        {
            uint32_t newEntry = physicalAddr & PAGE_TABLE_ADDRESS;  // set address
            newEntry |= PAGE_TABLE_READ_WRITE | PAGE_TABLE_PRESENT; // set read/write and present bits
            if (user)
            {
                newEntry |= PAGE_TABLE_USER; // set user privilege
            }

            pageTable[idx] = newEntry;

            virtualAddr = (pageDirIdx << 22) | (idx << 12);
            return true;
        }
    }

    return false;
}

void unmapPage(uint32_t* pageTable, uint32_t virtualAddr)
{
    // calculate the page table index
    int pageTableIdx = (virtualAddr >> 12) & PAGE_TABLE_INDEX_MASK;

    // clear the page table entry
    pageTable[pageTableIdx] = 0;

    // invalidate page in TLB
    invalidatePage(virtualAddr);
}

void mapModules(const multiboot_info* mbootInfo)
{
    uint32_t modAddr = mbootInfo->mods_addr + KERNEL_VIRTUAL_BASE;
    for (uint32_t i = 0; i < mbootInfo->mods_count; ++i)
    {
        const multiboot_mod_list* module = reinterpret_cast<const multiboot_mod_list*>(modAddr);

        uint32_t startPageAddr = align(module->mod_start, PAGE_SIZE, false);

        // end page is the page after the last page the module is in
        uint32_t endPageAddr = align(module->mod_end, PAGE_SIZE);

        for (uint32_t pageAddr = startPageAddr; pageAddr < endPageAddr; pageAddr += PAGE_SIZE)
        {
            uint32_t virtualAddr = pageAddr + KERNEL_VIRTUAL_BASE;
            mapPage(getKernelPageTableStart(), virtualAddr, pageAddr);
        }

        modAddr += sizeof(multiboot_mod_list);
    }
}
