#include "memmgr.h"
#include "pageframemgr.h"

MemMgr::MemMgr(PageFrameMgr* pageFrameManager, uintptr_t heapStartAddr)
{
    head = nullptr;
    pageFrameMgr = pageFrameManager;
    setHeapStart(heapStartAddr);
}

void MemMgr::setHeapStart(uintptr_t heapStartAddr)
{
    /// @todo verify this is page aligned
    heapStart = heapStartAddr;
}

void* MemMgr::alloc(size_t size)
{
    uintptr_t addr = heapStart;
    MemBlockInfo** ptr = &head;
    while (*ptr != nullptr)
    {
        MemBlockInfo& node = **ptr;

        addr = node.startAddr + node.size;
        ptr = &(node.nextNode);
    }

    /// @todo determine if we need to allocate a page

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
