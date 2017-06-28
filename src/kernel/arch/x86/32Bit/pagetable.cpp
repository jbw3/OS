#include <pageframemgr.h>
#include <pagetable.h>
#include <paging.h>

namespace mem {

PageTable::PageTable(PageFrameMgr* pfMgr, uint32_t* pageDir, uint16_t pageDirIdx)
    : _pageDir(pageDir),
    _pageDirIdx(pageDirIdx),
    _pfMgr(pfMgr)
{
    _pageTable = getPageTablePointer(pageDir, pageDirIdx);
}

bool PageTable::isFull()
{
    // cls: start at end for speed - loop will execute 1x
    // for page tables filled from front to back

    for (int16_t i = PAGE_TABLE_NUM_ENTRIES-1; i >= 0; i--)
    {
        if (!(_pageTable[i] & PAGE_TABLE_PRESENT))
        {
            return false;   // found an available entry
        }
    }
    return true;    // each entry was used
}

bool PageTable::isEmpty()
{
    for (uint16_t i = 0; i < PAGE_TABLE_NUM_ENTRIES; i++)
    {
        if (_pageTable[i] & PAGE_TABLE_PRESENT)
        {
            return false;   // found an active entry
        }
    }
    return true;    // each entry was inactive
}

uint16_t PageTable::nextAvailablePage()
{
    for (uint16_t i = 0; i < PAGE_TABLE_NUM_ENTRIES; i++)
    {
        if (!(_pageTable[i] & PAGE_TABLE_PRESENT))
        {
            return i;   // this entry is available
        }
    }
    return PAGE_TABLE_NUM_ENTRIES;  // invalid index
}

uint32_t PageTable::mapNextAvailablePageToAddress(uint32_t physAddr)
{
    uint16_t pageIdx = nextAvailablePage(); // get next available page index
    uint16_t offset = physAddr & 0xFFF;     // 12 bits offset
    uint32_t pageAddress = virtAddressOfPage(pageIdx);
    mapPage(_pageDir, pageAddress, physAddr);
    _pfMgr->reservePageFrame(physAddr);    // mark as reserved for page frame manager
    return pageAddress + offset;    // return virt address now mapped to physAddr
}

uint32_t PageTable::virtAddressOfPage(uint16_t pageIdx)
{
    uint32_t address = (_pageDirIdx & 0x3FF) << 22; // 10 bits of page dir index
    address |= (pageIdx & 0x3FF) << 12;             // 10 bits of page table index
    return address;
}

uint32_t PageTable::physAddressOfPageFrame(uint16_t pageIdx)
{
    return _pageTable[pageIdx] & PAGE_TABLE_ADDRESS;
}

bool PageTable::isMapped(uint32_t physAddr, uint32_t& virtAddr)
{
    for (uint16_t i = 0; i < PAGE_TABLE_NUM_ENTRIES; i++)
    {
        if (_pageTable[i] & PAGE_TABLE_PRESENT)
        {
            // get physical bounds of page frame
            uint32_t pageFrameAddress = physAddressOfPageFrame(i);
            uint32_t maxPageFrameAddr = pageFrameAddress + 0xFFF;

            // does the physical address fall within this page frame?
            if (physAddr >= pageFrameAddress && physAddr <= maxPageFrameAddr)
            {
                // set the corresponding virtual address
                auto offset = physAddr & 0xFFF;
                auto virtBase = (_pageDirIdx & 0x3FF) << 22;    // 10 bits of page dir index
                virtBase |= (i & 0x3FF) << 12;                  // 10 bits of page table index
                virtAddr = virtBase | offset;
                return true;
            }
        }
    }
    return false;   // no mappings found
}

}