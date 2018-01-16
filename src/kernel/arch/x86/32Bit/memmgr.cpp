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

    uintptr_t newNodeAddr = heapStart;
    MemBlockInfo* prevNode = head;
    MemBlockInfo* nextNode = nullptr;
    if (prevNode != nullptr)
    {
        nextNode = head->nextNode;
        while (nextNode != nullptr)
        {
            newNodeAddr = prevNode->startAddr + prevNode->size;

            // check if a new node can be added between 2 existing nodes
            if (newNodeAddr + totalSize <= reinterpret_cast<uintptr_t>(nextNode))
            {
                break;
            }

            prevNode = nextNode;
            nextNode = nextNode->nextNode;
        }
    }

    // determine if we need to allocate pages
    if (newNodeAddr + totalSize > heapEnd)
    {
        bool ok = allocPages(totalSize);

        // return null if we could not allocated enough pages
        if (!ok)
        {
            return nullptr;
        }
    }

    // allocate new node
    MemBlockInfo* newNode = reinterpret_cast<MemBlockInfo*>(newNodeAddr);
    newNode->startAddr = newNodeAddr + sizeof(MemBlockInfo);
    newNode->size = size;
    newNode->nextNode = nextNode;

    // add new node in the list
    if (prevNode == nullptr)
    {
        head = newNode;
    }
    else
    {
        prevNode->nextNode = newNode;
    }

    return reinterpret_cast<void*>(newNode->startAddr);
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

bool MemMgr::allocPages(size_t memSize)
{
    // calculate number of pages to allocate
    size_t numPages = memSize / PAGE_SIZE;
    if (memSize % PAGE_SIZE != 0)
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
    }

    return ok;
}
