/**
 * @brief Memory manager
 */

#ifndef MEM_MGR_H_
#define MEM_MGR_H_

#include <stddef.h>
#include <stdint.h>

class PageFrameMgr;

class MemMgr
{
public:
    /**
     * @brief Constructor
     */
    MemMgr(PageFrameMgr* pageFrameManager, uintptr_t heapStartAddr);

    void* alloc(size_t size);

    void free(void* ptr);

private:
    /**
     * @brief Information about an allocated block
     * of memory
     */
    struct MemBlockInfo
    {
        /// physical starting address of the memory block
        uintptr_t startAddr;

        /// size of the memory block in bytes
        size_t size;

        /// next node in the linked list
        MemBlockInfo* nextNode;
    };

    /// first node of linked list
    MemBlockInfo* head;

    /// page frame manager
    PageFrameMgr* pageFrameMgr;

    /// virtual start address of heap
    uintptr_t heapStart;

    /// virtual end address of heap
    uintptr_t heapEnd;
};

#endif // MEM_MGR_H_
