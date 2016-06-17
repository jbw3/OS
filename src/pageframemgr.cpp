/**
 * @brief Page frame manager
 */

#include "multiboot.h"
#include "pageframemgr.h"
#include "system.h"

/// @todo temporary
#include "screen.h"

PageFrameMgr::PageFrameMgr(const multiboot_info* mbootInfo)
{
    /// @todo pre-process mem map; get # of page frame blocks and mem segment
    /// containing the end of the kernel

    /// @todo allocate all needed page frame blocks at the end of the kernel

    /// @todo mark kernel in page frame blocks

    uint32_t mmapAddr = mbootInfo->mmap_addr + KERNEL_VIRTUAL_BASE;
    uint32_t mmapLen = mbootInfo->mmap_length;
    uint32_t offset = 0;
    const multiboot_mmap_entry* entry = reinterpret_cast<const multiboot_mmap_entry*>(mmapAddr + offset);
    while (offset < mmapLen)
    {
        uint64_t startAddr = entry->addr;
        uint64_t endAddr = entry->addr + entry->len - 1;

        offset += entry->size + sizeof(entry->size);
    }

    /// @todo ensure this is in a block
    /// @todo map if necessary
    firstBlock = reinterpret_cast<PageFrameBlock*>(KERNEL_VIRTUAL_END);
}
