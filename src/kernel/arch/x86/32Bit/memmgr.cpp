#include "memmgr.h"
#include "pageframemgr.h"
#include "pagetree.h"
#include "paging.h"
#include "screen.h"

MemMgr::MemMgr(uintptr_t heapStartAddr)
{
    head = nullptr;

    /// @todo verify this is page aligned
    heapStart = heapEnd = heapStartAddr;
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
            // check if a new node can be added between 2 existing nodes
            if (prevNode->startAddr + prevNode->size + totalSize <= reinterpret_cast<uintptr_t>(nextNode))
            {
                break;
            }

            prevNode = nextNode;
            nextNode = nextNode->nextNode;
        }

        /// @todo align on 4-byte boundary
        newNodeAddr = prevNode->startAddr + prevNode->size;
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
                pageFrameMgr.freePageFrame(phyPageAddr);
            }
            heapEnd -= PAGE_SIZE;
        }
    }

    return ok;
}

void MemMgr::printBlocks()
{
    if (head == nullptr)
    {
        screen << "<No memory allocated>\n";
    }
    else
    {
        MemBlockInfo* node = head;
        while (node != nullptr)
        {
            screen << os::Screen::hex << os::Screen::setfill('0')
                << os::Screen::setw(8) << node->startAddr
                << " - "
                << os::Screen::setw(8) << (node->startAddr + node->size)
                << os::Screen::dec << os::Screen::setfill(' ')
                << " (" << node->size << ")\n";

            node = node->nextNode;
        }
    }
}
