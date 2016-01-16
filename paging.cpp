#include "paging.h"
#include "screen.h"
#include "system.h"

namespace
{

/**
 * @brief Identity map the kernel
 * @todo Add all pages in the first page table for now. Later,
 * this should only map the kernel code and data
 */
void mapKernel()
{
    /*
    uint32_t kStart = reinterpret_cast<uint32_t>(getKImgStart());
    uint32_t kEnd = reinterpret_cast<uint32_t>(getKImgEnd());
    for (uint32_t pageAddr = kStart; pageAddr < kEnd; pageAddr += 4096)
    {
        addPage(pageAddr);
    }
    */
    for (uint32_t i = 0; i < 1024; ++i)
    {
        addPage(i * 4096);
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

    // add a page table
    initPageTable(0x200000);
    addPageTable(0, 0x200000);
    addPage(0x200000);

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
