#include "paging.h"
#include "pagetreex86_32.h"
#include "system.h"

PageTreeX86_32::PageTreeX86_32(uintptr_t pageDirAddr)
{
    pageDir = reinterpret_cast<uintptr_t*>(pageDirAddr);
}

bool PageTreeX86_32::map(uintptr_t virtualAddr, uintptr_t physicalAddr, unsigned int flags)
{
    // calculate the page dir index
    int pageDirIdx = virtualAddr & PAGE_DIR_INDEX_MASK;

    // get the page dir entry
    uint32_t pageDirEntry = pageDir[pageDirIdx];

    // make sure the page table is present
    if ( (pageDirEntry & PAGE_DIR_PRESENT) == 0 )
    {
        PANIC("TODO: Map page table.");
    }

    // get the page table
    uint32_t pageTableAddr = pageDirEntry & PAGE_DIR_ADDRESS;
    uint32_t* pageTable = reinterpret_cast<uint32_t*>(pageTableAddr);

    // calculate the page table index
    int pageTableIdx = (virtualAddr >> PAGE_TABLE_INDEX_SHIFT) & PAGE_TABLE_INDEX_MASK;

    // create the page table entry
    uint32_t pageTableEntry = 0;
    pageTableEntry |= physicalAddr & PAGE_TABLE_ADDRESS; // add new address
    pageTableEntry |= PAGE_TABLE_PRESENT;                // set present bit

    if ( (flags & eUser) != 0 )
    {
        pageTableEntry |= PAGE_TABLE_USER; // set user privilege
    }

    if ( (flags & eReadWrite) != 0 )
    {
        pageTableEntry |= PAGE_TABLE_READ_WRITE; // set read/write
    }

    // insert the entry into the page table
    pageTable[pageTableIdx] = pageTableEntry;

    return true;
}

bool PageTreeX86_32::mapOnOrAfter(uintptr_t startAddr, uintptr_t& virtualAddr, uintptr_t physicalAddr, unsigned int flags)
{
}

bool PageTreeX86_32::mapOnOrBefore(uintptr_t startAddr, uintptr_t& virtualAddr, uintptr_t physicalAddr, unsigned int flags)
{
}

void PageTreeX86_32::unmap(uintptr_t virtualAddr)
{
    // calculate the page dir index
    int pageDirIdx = virtualAddr & PAGE_DIR_INDEX_MASK;

    // get the page dir entry
    uint32_t pageDirEntry = pageDir[pageDirIdx];

    // make sure the page table is present
    if ( (pageDirEntry & PAGE_DIR_PRESENT) != 0 )
    {
        // get the page table
        uint32_t pageTableAddr = pageDirEntry & PAGE_DIR_ADDRESS;
        uint32_t* pageTable = reinterpret_cast<uint32_t*>(pageTableAddr);

        // calculate the page table index
        int pageTableIdx = (virtualAddr >> PAGE_TABLE_INDEX_SHIFT) & PAGE_TABLE_INDEX_MASK;

        // clear the page table entry
        pageTable[pageTableIdx] = 0;

        // invalidate page in TLB
        invalidatePage(virtualAddr);
    }
}
