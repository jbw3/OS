#include "paging.h"
#include "pagetreex86_32.h"
#include "system.h"

PageTree::Entry* const PageTreeX86_32::KERNEL_PAGE_TABLES = reinterpret_cast<Entry* const>(8_MiB);
PageTree::Entry* const PageTreeX86_32::PROCESS_PAGE_TABLES = PageTreeX86_32::KERNEL_PAGE_TABLES + (PAGE_DIR_NUM_ENTRIES * PAGE_SIZE);

void PageTreeX86_32::init()
{
    /// @todo set up the kernel page tables
}

PageTreeX86_32::PageTreeX86_32(uintptr_t pageDirAddr, bool isKernel) :
    pageTables(isKernel ? KERNEL_PAGE_TABLES : PROCESS_PAGE_TABLES)
{
    pageDir = reinterpret_cast<Entry*>(pageDirAddr);
}

bool PageTreeX86_32::map(uintptr_t virtualAddr, uintptr_t physicalAddr, unsigned int flags)
{
    // calculate the page dir index
    int pageDirIdx = virtualAddr & PAGE_DIR_INDEX_MASK;

    // get the page dir entry
    Entry pageDirEntry = pageDir[pageDirIdx];

    // make sure the page table is present
    if ( (pageDirEntry & PAGE_DIR_PRESENT) == 0 )
    {
        PANIC("TODO: Map page table.");
        return false;
    }

    // get the page table
    Entry* pageTable = getPageTable(pageDirIdx);

    // calculate the page table index
    int pageTableIdx = (virtualAddr >> PAGE_TABLE_INDEX_SHIFT) & PAGE_TABLE_INDEX_MASK;

    // create the page table entry
    Entry pageTableEntry = 0;
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
    PANIC("Not implemented.");
}

bool PageTreeX86_32::mapOnOrBefore(uintptr_t startAddr, uintptr_t& virtualAddr, uintptr_t physicalAddr, unsigned int flags)
{
    PANIC("Not implemented.");
}

void PageTreeX86_32::unmap(uintptr_t virtualAddr)
{
    // calculate the page dir index
    int pageDirIdx = virtualAddr & PAGE_DIR_INDEX_MASK;

    // get the page dir entry
    Entry pageDirEntry = pageDir[pageDirIdx];

    // make sure the page table is present
    if ( (pageDirEntry & PAGE_DIR_PRESENT) != 0 )
    {
        // get the page table
        Entry* pageTable = getPageTable(pageDirIdx);

        // calculate the page table index
        int pageTableIdx = (virtualAddr >> PAGE_TABLE_INDEX_SHIFT) & PAGE_TABLE_INDEX_MASK;

        // clear the page table entry
        pageTable[pageTableIdx] = 0;

        // invalidate page in TLB
        invalidatePage(virtualAddr);
    }
}

PageTree::Entry* PageTreeX86_32::getPageTable(int pageDirIdx) const
{
    return pageTables + (pageDirIdx * PAGE_SIZE);
}
