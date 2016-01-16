#include "paging.h"
#include "screen.h"
#include "system.h"

namespace
{

/**
 * @brief Identity map the kernel
 */
void mapKernel()
{
    for (uint32_t addr = 0; addr < getKernelEnd(); addr += 4096)
    {
        addPage(addr);
    }
}

}

extern "C"
void pageFault(const registers* /*regs*/)
{
    os::Screen::EColor bgColor = screen.getBackgroundColor();
    os::Screen::EColor fgColor = screen.getForegroundColor();

    screen.setBackgroundColor(os::Screen::EColor::eRed);
    screen.setForegroundColor(os::Screen::EColor::eWhite);

    screen << "Page fault!";

    screen.setBackgroundColor(bgColor);
    screen.setForegroundColor(fgColor);

    // hang
    while (true);
}

void initPaging()
{
    // register page fault handler
    registerIsrHandler(ISR_PAGE_FAULT, pageFault);

    // initialize the page driectory
    initPageDir();

    // add a page table after the kernel
    uint32_t pageAddr = getKernelEnd();
    if ((pageAddr & 0xFFF) != 0)
    {
        pageAddr &= 0xFFFF'F000;
        pageAddr += 4096;
    }
    initPageTable(pageAddr);
    addPageTable(0, pageAddr);

    mapKernel();

    enablePaging();
}

void addPageTable(int idx, uint32_t pageTableAddr)
{
    uint32_t* pageDir = getPageDirStart();

    // get the page directory entry from the page directory
    uint32_t pageDirEntry = pageDir[idx];

    // add the page table address to the page directory
    pageDirEntry &= ~PAGE_DIR_ADDRESS;
    pageDirEntry |= (pageTableAddr & PAGE_DIR_ADDRESS);
    pageDirEntry |= PAGE_DIR_PRESENT;
    pageDir[idx] = pageDirEntry;
}

void addPage(uint32_t pageAddr)
{
    uint32_t* pageDir = getPageDirStart();

    // calculate the page directory and page table indexes
    int pageDirIdx = pageAddr >> 22;
    int pageTableIdx = (pageAddr >> 12) & 0x3FF;

    // get the page table address from the page directory
    uint32_t pageDirEntry = pageDir[pageDirIdx];
    uint32_t* pageTable = reinterpret_cast<uint32_t*>(pageDirEntry & PAGE_DIR_ADDRESS);

    // get the page address from the page table
    uint32_t pageTableEntry = pageTable[pageTableIdx];

    // add the page address to the page table
    pageTableEntry &= ~PAGE_TABLE_ADDRESS;
    pageTableEntry |= (pageAddr & PAGE_TABLE_ADDRESS);
    pageTableEntry |= PAGE_TABLE_PRESENT;
    pageTable[pageTableIdx] = pageTableEntry;
}
