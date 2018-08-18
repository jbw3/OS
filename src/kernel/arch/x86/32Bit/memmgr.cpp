#include <string.h>

#include "memmgr.h"
#include "pageframemgr.h"
#include "pagetree.h"
#include "paging.h"
#include "utils.h"
#include "userlogger.h"

/// @todo pass in PageTree pointer
MemMgr::MemMgr(uintptr_t heapStartAddr)
{
    head = nullptr;

    // page align the heap starting address
    heapStart = heapEnd = align(heapStartAddr, PAGE_SIZE);
}

MemMgr::~MemMgr()
{
    size_t numAllocatedPages = (heapEnd - heapStart) / PAGE_SIZE;
    freePages(numAllocatedPages);
}

void* MemMgr::alloc(size_t size)
{
    // calculate total size of allocated memory and info block
    size_t totalSize = size + sizeof(MemBlockInfo);

    /// @todo look for space before the head node

    uintptr_t newNodeAddr = heapStart;
    MemBlockInfo* prevNode = head;
    MemBlockInfo* nextNode = nullptr;
    if (prevNode != nullptr)
    {
        nextNode = prevNode->nextNode;
        while (nextNode != nullptr)
        {
            uintptr_t addr = align(prevNode->startAddr + prevNode->size, sizeof(uintptr_t));

            // check if a new node can be added between 2 existing nodes
            if (addr + totalSize <= reinterpret_cast<uintptr_t>(nextNode))
            {
                break;
            }

            prevNode = nextNode;
            nextNode = nextNode->nextNode;
        }

        // calculate new node address
        newNodeAddr = prevNode->startAddr + prevNode->size;

        // align address on word boundary
        newNodeAddr = align(newNodeAddr, sizeof(uintptr_t));
    }

    // determine if we need to allocate pages
    if (newNodeAddr + totalSize > heapEnd)
    {
        size_t pageAllocSize = totalSize - (heapEnd - newNodeAddr);
        bool ok = allocPages(pageAllocSize);

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

    // remove memory from linked list
    MemBlockInfo* prevNode = nullptr;
    MemBlockInfo* node = head;
    while (node != nullptr)
    {
        if (node->startAddr == reinterpret_cast<uintptr_t>(ptr))
        {
            if (prevNode == nullptr)
            {
                head = node->nextNode;
            }
            else
            {
                prevNode->nextNode = node->nextNode;
            }

            break;
        }

        prevNode = node;
        node = node->nextNode;
    }

    /// @todo free page frames if able
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
        /// @todo How do we get access to the pageFrameMgr in a process? Need a system call.
        uintptr_t phyPageAddr = pageFrameMgr.allocPageFrame();

        if (phyPageAddr == 0)
        {
            ok = false;
        }
        else
        {
            ++numPagesAllocated;

            // map new page in heap
            ok = PageTree::getKernelPageTree().map(heapEnd, phyPageAddr, PageTree::eReadWrite);

            // clear memory
            memset(reinterpret_cast<void*>(heapEnd), 0, PAGE_SIZE);

            heapEnd += PAGE_SIZE;
        }
    }

    // if something went wrong, deallocate the pages just allocated
    if (!ok)
    {
        freePages(numPagesAllocated);
    }

    return ok;
}

void MemMgr::freePages(size_t numPages)
{
    for (size_t i = 0; i < numPages; ++i)
    {
        heapEnd -= PAGE_SIZE;

        uintptr_t phyPageAddr = 0;
        bool found = PageTree::getKernelPageTree().getPhysicalAddress(heapEnd, phyPageAddr);
        if (found)
        {
            // unmap the page
            PageTree::getKernelPageTree().unmap(heapEnd);

            // free the page
            pageFrameMgr.freePageFrame(phyPageAddr);
        }
    }
}

void MemMgr::printBlocks()
{
    if (head == nullptr)
    {
        ulog.log("<No memory allocated>\n");
    }
    else
    {
        MemBlockInfo* node = head;
        while (node != nullptr)
        {
            ulog.log("{x0>8} - {x0>8} ({})\n", node->startAddr, (node->startAddr + node->size), node->size);

            node = node->nextNode;
        }
    }
}
