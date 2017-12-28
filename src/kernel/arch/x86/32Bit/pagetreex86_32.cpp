#include <string.h>
#include "pageframemgr.h"
#include "pagetreex86_32.h"
#include "paging.h"
#include "system.h"

PageTree::Entry* const PageTreeX86_32::KERNEL_PAGE_TABLES = reinterpret_cast<Entry* const>(KERNEL_VIRTUAL_BASE + 8_MiB);
PageTree::Entry* const PageTreeX86_32::PROCESS_PAGE_TABLES = PageTreeX86_32::KERNEL_PAGE_TABLES + (PAGE_DIR_NUM_ENTRIES * PAGE_SIZE);

PageTree* PageTree::kernelPageTree = nullptr;

PageTreeX86_32::PageTreeX86_32(uintptr_t pageDirAddr, bool isKernel) :
    pageTables(isKernel ? KERNEL_PAGE_TABLES : PROCESS_PAGE_TABLES)
{
    pageDir = reinterpret_cast<Entry*>(pageDirAddr);

    if (isKernel)
    {
        kernelPageTree = this;

        // map a page table for the kernel page tables
        int pageDirIdx = (reinterpret_cast<uintptr_t>(KERNEL_PAGE_TABLES) >> PAGE_DIR_INDEX_SHIFT) & PAGE_DIR_INDEX_MASK;

        // map a page table to let us access the other page tables
        // (the page table page table)
        uintptr_t* pageTablePageTable = getKernelPageTable2();
        memset(pageTablePageTable, 0, PAGE_SIZE); // clear page table

        uintptr_t virtualAddr = reinterpret_cast<uintptr_t>(pageTablePageTable);
        uintptr_t physicalAddr = virtualAddr - KERNEL_VIRTUAL_BASE;
        Entry pageDirEntry = 0;
        pageDirEntry |= physicalAddr & PAGE_DIR_ADDRESS;
        pageDirEntry |= PAGE_DIR_READ_WRITE | PAGE_DIR_PRESENT;
        pageDir[pageDirIdx] = pageDirEntry;

        // map the page table page table in itself
        virtualAddr = reinterpret_cast<uintptr_t>(getPageTable(pageDirIdx));
        int pageTableIdx = (reinterpret_cast<uintptr_t>(virtualAddr) >> PAGE_TABLE_INDEX_SHIFT) & PAGE_TABLE_INDEX_MASK;
        Entry pageTableEntry = 0;
        pageTableEntry |= physicalAddr & PAGE_TABLE_ADDRESS;
        pageTableEntry |= PAGE_TABLE_READ_WRITE | PAGE_TABLE_PRESENT;
        pageTablePageTable[pageTableIdx] = pageTableEntry;

        // map the existing kernel page table
        pageDirIdx = (KERNEL_VIRTUAL_START >> PAGE_DIR_INDEX_SHIFT) & PAGE_DIR_INDEX_MASK;
        virtualAddr = reinterpret_cast<uintptr_t>(getPageTable(pageDirIdx));
        physicalAddr = reinterpret_cast<uintptr_t>(getKernelPageTable1()) - KERNEL_VIRTUAL_BASE;
        map(virtualAddr, physicalAddr, eReadWrite);
    }
}

bool PageTreeX86_32::map(uintptr_t virtualAddr, uintptr_t physicalAddr, unsigned int flags)
{
    // calculate the page dir index
    int pageDirIdx = (virtualAddr >> PAGE_DIR_INDEX_SHIFT) & PAGE_DIR_INDEX_MASK;

    // get the page dir entry
    Entry pageDirEntry = pageDir[pageDirIdx];

    // check if the page table is present
    if ( (pageDirEntry & PAGE_DIR_PRESENT) == 0 )
    {
        // create a new page table

        // allocate a page frame for the new page table
        uintptr_t ptPhyAddr = pageFrameMgr.allocPageFrame();
        if (ptPhyAddr == 0)
        {
            return false;
        }

        // add a new page directory entry
        pageDirEntry = 0;
        pageDirEntry |= ptPhyAddr & PAGE_DIR_ADDRESS;
        pageDirEntry |= PAGE_DIR_READ_WRITE | PAGE_DIR_PRESENT;
        pageDir[pageDirIdx] = pageDirEntry;

        // map the new page table
        Entry* pt = getPageTable(pageDirIdx);
        uintptr_t ptVirAddr = reinterpret_cast<uintptr_t>(pt);
        map(ptVirAddr, ptPhyAddr, eReadWrite);

        // clear the new page table
        memset(pt, 0, PAGE_SIZE);
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
    int pageDirIdx = (virtualAddr >> PAGE_DIR_INDEX_SHIFT) & PAGE_DIR_INDEX_MASK;

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
    return pageTables + (pageDirIdx * PAGE_SIZE / sizeof(Entry));
}
