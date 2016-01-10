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
    uint32_t kStart = reinterpret_cast<uint32_t>(getKImgStart());
    uint32_t kEnd = reinterpret_cast<uint32_t>(getKImgEnd());
    for (uint32_t pageAddr = kStart; pageAddr < kEnd; pageAddr += 4096)
    {
        addPage(pageAddr);
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
    installIrqHandler(IRQ_PAGE_FAULT, pageFault);

    // initialize the page driectory
    initPageDir();

    // add a page table
    initPageTable(0);
    addPageTable(0);
    addPage(0);

    mapKernel();

    // enablePaging();
}

void addPageTable(uint32_t pageTableAddr)
{
    uint32_t* pageDir = getPageDirStart();

    // calculate the page directory index
    int pageDirIdx = pageTableAddr >> 22;

    // get the page directory entry from the page directory
    uint32_t pageDirEntry = pageDir[pageDirIdx];

    // add the page table address to the page directory
    pageDirEntry &= ~PAGE_DIR_ADDRESS;
    pageDirEntry |= (pageTableAddr & PAGE_DIR_ADDRESS);
    pageDirEntry |= PAGE_DIR_PRESENT;
    pageDir[pageDirIdx] = pageDirEntry;
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
