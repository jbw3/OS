/**
 * @brief Memory manager
 */

#include <stddef.h>
#include <stdint.h>

class MemMgr
{
public:
    /**
     * @brief Constructor
     */
    MemMgr();

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
};
