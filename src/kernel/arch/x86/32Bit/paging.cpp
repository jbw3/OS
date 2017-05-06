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

    // hang
    while (true);
}

void configPaging()
{
    // register page fault handler
    registerIsrHandler(ISR_PAGE_FAULT, pageFault);
}

void addPageTable(int idx, uint32_t pageTableAddr)
{
    uint32_t* pageDir = getKernelPageDirStart();

    // get the page directory entry from the page directory
    uint32_t pageDirEntry = pageDir[idx];

    // add the page table address to the page directory
    pageDirEntry &= ~PAGE_DIR_ADDRESS;                      // clear previous address
    pageDirEntry |= pageTableAddr & PAGE_DIR_ADDRESS;       // add new address
    pageDirEntry |= PAGE_DIR_READ_WRITE | PAGE_DIR_PRESENT; // set read/write and present bits
    pageDir[idx] = pageDirEntry;
}

void addPage(uint32_t pageAddr)
{
    uint32_t* pageDir = getKernelPageDirStart();

    // calculate the page directory and page table indexes
    int pageDirIdx = pageAddr >> 22;
    int pageTableIdx = (pageAddr >> 12) & 0x3FF;

    // get the page table address from the page directory
    uint32_t pageDirEntry = pageDir[pageDirIdx];
    uint32_t* pageTable = reinterpret_cast<uint32_t*>(pageDirEntry & PAGE_DIR_ADDRESS);

    // get the page address from the page table
    uint32_t pageTableEntry = pageTable[pageTableIdx];

    // add the page address to the page table
    pageTableEntry &= ~PAGE_TABLE_ADDRESS;                        // clear previous address
    pageTableEntry |= pageAddr & PAGE_TABLE_ADDRESS;              // add new address
    pageTableEntry |= PAGE_TABLE_READ_WRITE | PAGE_TABLE_PRESENT; // set read/write and present bits
    pageTable[pageTableIdx] = pageTableEntry;
}

void mapPage(const uint32_t* pageDir, uint32_t virtualAddr, uint32_t physicalAddr)
{
    // calculate the page directory and page table indexes
    int pageDirIdx = virtualAddr >> 22;
    int pageTableIdx = (virtualAddr >> 12) & PAGE_SIZE_MASK;

    // get the page table address from the page directory
    uint32_t pageDirEntry = pageDir[pageDirIdx];
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