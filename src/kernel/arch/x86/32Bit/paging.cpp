#include "paging.h"
#include "screen.h"
#include "system.h"

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

void mapPage(const uint32_t* pageDir, uint32_t virtualAddr, uint32_t physicalAddr)
{
    // calculate the page directory and page table indexes
    int pageDirIdx = virtualAddr >> 22;
    int pageTableIdx = (virtualAddr >> 12) & PAGE_TABLE_INDEX_MASK;

    // get the entry in the page directory
    uint32_t pageDirEntry = pageDir[pageDirIdx];

    // make sure the entry contains a page table
    if ( (pageDirEntry & PAGE_DIR_PRESENT) == 0 )
    {
        PANIC("Page directory entry does not point to a page table.");
    }

    // get the page table address from the page directory
    uint32_t pageTableAddr = pageDirEntry & PAGE_DIR_ADDRESS;
    uint32_t* pageTable = reinterpret_cast<uint32_t*>(pageTableAddr + KERNEL_VIRTUAL_BASE);

    // get the page entry from the page table
    uint32_t pageTableEntry = pageTable[pageTableIdx];

    // add the page address to the page table
    pageTableEntry &= ~PAGE_TABLE_ADDRESS;                        // clear previous address
    pageTableEntry |= physicalAddr & PAGE_TABLE_ADDRESS;          // add new address
    pageTableEntry |= PAGE_TABLE_READ_WRITE | PAGE_TABLE_PRESENT; // set read/write and present bits
    pageTable[pageTableIdx] = pageTableEntry;
}
