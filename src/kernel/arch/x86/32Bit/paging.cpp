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
           << "Error code: " << regs->errCode << '\n'
           << "Address: "
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

/**
 * @brief Find a page's info. This is a helper function for functions below.
 */
void findPage(const uint32_t* pageDir, uint32_t virtualAddr, int& pageTableIdx, bool& foundPageTable, uint32_t*& pageTable)
{
    // calculate the page directory and page table indexes
    int pageDirIdx = virtualAddr >> 22;
    pageTableIdx = (virtualAddr >> 12) & PAGE_TABLE_INDEX_MASK;

    // get the entry in the page directory
    uint32_t pageDirEntry = pageDir[pageDirIdx];

    foundPageTable = (pageDirEntry & PAGE_DIR_PRESENT) != 0;

    // if no page table is mapped, return
    if (!foundPageTable)
    {
        pageTable = nullptr;
        return;
    }

    // get the page table address from the page directory
    uint32_t pageTableAddr = pageDirEntry & PAGE_DIR_ADDRESS;
    pageTable = reinterpret_cast<uint32_t*>(pageTableAddr + KERNEL_VIRTUAL_BASE);
}

void mapPage(const uint32_t* pageDir, uint32_t virtualAddr, uint32_t physicalAddr, bool user)
{
    int pageTableIdx = -1;
    bool foundPageTable = false;
    uint32_t* pageTable = nullptr;

    findPage(pageDir, virtualAddr, pageTableIdx, foundPageTable, pageTable);

    // make sure the entry contains a page table
    if (!foundPageTable)
    {
        PANIC("Page directory entry does not point to a page table.");
    }

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

void unmapPage(const uint32_t* pageDir, uint32_t virtualAddr)
{
    int pageTableIdx = -1;
    bool foundPageTable = false;
    uint32_t* pageTable = nullptr;

    findPage(pageDir, virtualAddr, pageTableIdx, foundPageTable, pageTable);

    // clear the page if the page table was found, else do nothing
    if (foundPageTable)
    {
        // clear the page table entry
        pageTable[pageTableIdx] = 0;

        // invalidate page in TLB
        invalidatePage(virtualAddr);
    }
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
            mapPage(getKernelPageDirStart(), virtualAddr, pageAddr);
        }

        modAddr += sizeof(multiboot_mod_list);
    }
}
