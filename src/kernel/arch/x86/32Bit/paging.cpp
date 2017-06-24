#include "paging.h"
#include "screen.h"
#include "system.h"

/**
 * @brief Starting index of kernel page directory entries (3GB and above
 * in virtual address space)
 */
const uint16_t KERNEL_PAGEDIR_START_IDX = 0x300;

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
    screen << os::Screen::hex;
    //screen << "\n\n";
    // screen << "VIRTUAL ADDRESS: 0x" << virtualAddr << "\n";
    // screen << "PHYSICAL ADDRESS: 0x" << physicalAddr << "\n";
    // screen << "PAGE DIR INDEX: 0x" << pageDirIdx << "\n";
    // screen << "PAGE DIR ENTRY: 0x" << pageDirEntry << "\n";

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

/**
 * @brief Returns a pointer that can be used to access the page table
 * identified by the given index in the given page directory.
 * since the PDE stores a physical address of the page table,
 * this function transforms this address into a virtual one that
 * can be used directly.
 */
uint32_t* getPageTablePointer(uint32_t* pageDir, uint16_t index)
{
    uint32_t physicalPTAddress = (pageDir[index] & PAGE_DIR_ADDRESS);
    return reinterpret_cast<uint32_t*>(physicalPTAddress + KERNEL_VIRTUAL_BASE);
}

namespace pageInfo {

static uint32_t* _kPageDir = getKernelPageDirStart();

uint32_t kNumAvailablePDEs()
{
    return numAvailablPDEs(_kPageDir, KERNEL_PAGEDIR_START_IDX);
}

uint32_t numAvailablPDEs(uint32_t* pageDir, uint16_t startIdx)
{
    int numPageDirEntries = PAGE_DIR_NUM_ENTRIES;

    for (uint16_t i = startIdx; i < PAGE_DIR_NUM_ENTRIES; i++)
    {
        numPageDirEntries -= (pageDir[i] & PAGE_DIR_PRESENT);
    }

    return numPageDirEntries;
}

uint16_t lastUsedKernelPDEIndex()
{
    // returns index of last kernel PDE out of those currently in use
    for (uint16_t i = KERNEL_PAGEDIR_START_IDX; i < PAGE_DIR_NUM_ENTRIES; i++)
    {
        if (!(_kPageDir[i] & PAGE_DIR_PRESENT))
        {
            // PDE not present - the last index was the last one in use
            return i-1;
        }
    }

    return PAGE_DIR_NUM_ENTRIES-1;  // all in use
}

uint16_t lastUsedPTEIndex(uint32_t* pageDir, uint16_t pdeIndex)
{
    // returns index of last used PTE out of those currently in use
    uint32_t* pageTable = getPageTablePointer(pageDir, pdeIndex);

    for (uint16_t i = 0; i < PAGE_TABLE_NUM_ENTRIES; i++)
    {
        if (!(pageTable[i] & PAGE_TABLE_PRESENT))
        {
            // PTE not present - the last index was the last one in use

        }
    }
}

}