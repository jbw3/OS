/**
 * @brief Page frame manager
 */

#include "multiboot.h"
#include "pageframemgr.h"
#include "paging.h"
#include "system.h"

/// @todo temporary
#include "screen.h"

PageFrameMgr::PageFrameMgr(const multiboot_info* mbootInfo)
{
    // get number of page frames
    uint32_t numPageFrames = 0;
    getMultibootMMapInfo(mbootInfo, numPageFrames);

    screen << "numPageFrames = " << numPageFrames << '\n';

    /// @todo get mem segment containing the end of the kernel

    /// @todo allocate all needed page frame blocks at the end of the kernel

    /// @todo mark kernel in page frame blocks

    /// @todo ensure this is in a block
    /// @todo map if necessary
    firstBlock = reinterpret_cast<PageFrameBlock*>(KERNEL_VIRTUAL_END);
}

void PageFrameMgr::getMultibootMMapInfo(const multiboot_info* mbootInfo, uint32_t& numPageFrames)
{
    numPageFrames = 0;

    uint32_t mmapAddr = mbootInfo->mmap_addr + KERNEL_VIRTUAL_BASE;
    uint32_t mmapLen = mbootInfo->mmap_length;
    uint32_t offset = 0;
    while (offset < mmapLen)
    {
        const multiboot_mmap_entry* entry = reinterpret_cast<const multiboot_mmap_entry*>(mmapAddr + offset);
        uint64_t entryStart = entry->addr;
        uint64_t entryEnd = entry->addr + entry->len - 1;

        // skip map entries below 1 MiB
        if (entryEnd >= 0x10'0000)
        {
            // skip page frames below 1 MiB
            uint32_t pfStart = entryStart;
            if (pfStart < 0x10'0000)
            {
                pfStart = 0x10'0000;
            }

            // find nearest page frame boundary
            if ((pfStart & ~PAGE_SIZE_MASK) != 0)
            {
                pfStart += PAGE_SIZE;
                pfStart &= PAGE_SIZE_MASK;
            }

            // ensure the start is still within the mmap entry bounds
            if (pfStart <= entryEnd)
            {
                numPageFrames += (entryEnd + 1 - pfStart) / PAGE_SIZE;
            }
        }

        offset += entry->size + sizeof(entry->size);
    }
}
