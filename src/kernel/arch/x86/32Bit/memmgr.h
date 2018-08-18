#ifndef MEM_MGR_H_
#define MEM_MGR_H_

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Memory manager
 */
class MemMgr
{
public:
    /**
     * @brief Constructor.
     */
    MemMgr(uintptr_t heapStartAddr);

    /**
     * @brief Free all page frames.
     */
    ~MemMgr();

    void* alloc(size_t size);

    void free(void* ptr);

    void printBlocks();

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

    /// virtual start address of heap
    uintptr_t heapStart;

    /// virtual end address of heap
    uintptr_t heapEnd;

    /**
     * @brief Allocated pages and map them at the end of the heap.
     */
    bool allocPages(size_t memSize);

    /**
     * @brief Free pages from the end of the heap.
     */
    void freePages(size_t numPages);
};

#endif // MEM_MGR_H_
