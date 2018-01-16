#include "memmgr.h"
#include "pageframemgr.h"
#include "pagetree.h"
#include "paging.h"

MemMgr::MemMgr(PageFrameMgr* pageFrameManager, uintptr_t heapStartAddr)
{
    head = nullptr;
    pageFrameMgr = pageFrameManager;

    /// @todo verify this is page aligned
    heapStart = heapEnd = heapStartAddr;
}

void* MemMgr::alloc(size_t size)
{
    // calculate total size of allocated memory and info block
    size_t totalSize = size + sizeof(MemBlockInfo);

    uintptr_t addr = heapStart;
    MemBlockInfo** ptr = &head;
    while (*ptr != nullptr)
    {
        MemBlockInfo& node = **ptr;

        addr = node.startAddr + node.size;
        ptr = &(node.nextNode);
    }

    // determine if we need to allocate pages
    if (addr + totalSize > heapEnd)
    {
        // calculate number of pages to allocate
        size_t numPages = totalSize / PAGE_SIZE;
        if (totalSize % PAGE_SIZE != 0)
        {
            ++numPages;
        }

        bool ok = true;
        size_t numPagesAllocated = 0;
        while (numPagesAllocated < numPages && ok)
        {
            uintptr_t phyPageAddr = pageFrameMgr->allocPageFrame();

            if (phyPageAddr == 0)
            {
                ok = false;
            }
            else
            {
                ++numPagesAllocated;

                // map new page in heap
                ok = PageTree::getKernelPageTree().map(heapEnd, phyPageAddr, PageTree::eReadWrite);
                heapEnd += PAGE_SIZE;
            }
        }

        // if something went wrong, deallocate the pages just allocated
        // and return null
        if (!ok)
        {
            for (size_t i = 0; i < numPagesAllocated; ++i)
            {
                uintptr_t phyPageAddr = 0;
                bool found = PageTree::getKernelPageTree().getPhysicalAddress(heapEnd, phyPageAddr);
                if (found)
                {
                    pageFrameMgr->freePageFrame(phyPageAddr);
                }
                heapEnd -= PAGE_SIZE;
            }

            return nullptr;
        }
    }

    /// @todo allocate new node
}

void MemMgr::free(void* ptr)
{
    // do nothing if the pointer is null
    if (ptr == nullptr)
    {
        return;
    }

    /// @todo remove memory from linked list
}
