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
    constexpr unsigned int MAX_MEM_BLOCKS = 32;
    MemBlock memBlocks[MAX_MEM_BLOCKS];

    // initialize memory block data structure
    unsigned int numMemBlocks = 0;
    initMemBlocks(mbootInfo, memBlocks, MAX_MEM_BLOCKS, numMemBlocks);

    screen << "numMemBlocks  = " << numMemBlocks << '\n';
    for (unsigned int i = 0; i < numMemBlocks; ++i)
    {
        screen << os::Screen::hex
               << os::Screen::setw(8)
               << os::Screen::setfill('0')
               << memBlocks[i].startAddr
               << os::Screen::dec
               << os::Screen::setfill(' ')
               << "  " << memBlocks[i].numPages
               << '\n';
    }

    // get number of page frames
    uint32_t numPageFrames = 0;
    getMultibootMMapInfo(mbootInfo, numPageFrames);

    screen << "numPageFrames = " << numPageFrames << '\n';

    // allocate and initialize all needed page frame blocks at the end of the kernel
    initDataStruct(memBlocks, numMemBlocks);

    /// @todo get mem segment containing the end of the kernel

    /// @todo mark kernel in page frame blocks

    /// @todo ensure this is in a block
    /// @todo map if necessary
    // blocks = reinterpret_cast<PageFrameBlock*>(KERNEL_VIRTUAL_END);
}

void PageFrameMgr::initMemBlocks(const multiboot_info* mbootInfo, MemBlock* memBlocks, unsigned int memBlocksSize, unsigned int& numMemBlocks)
{
    numMemBlocks = 0;

    uint32_t mmapAddr = mbootInfo->mmap_addr + KERNEL_VIRTUAL_BASE;
    uint32_t mmapLen = mbootInfo->mmap_length;
    uint32_t offset = 0;
    while (offset < mmapLen && numMemBlocks < memBlocksSize)
    {
        const multiboot_mmap_entry* entry = reinterpret_cast<const multiboot_mmap_entry*>(mmapAddr + offset);
        uint64_t memBlockStart = entry->addr;
        uint64_t memBlockEnd = entry->addr + entry->len;

        // check if this entry is adjacent to the next entry
        offset += entry->size + sizeof(entry->size);
        while (offset < mmapLen)
        {
            entry = reinterpret_cast<const multiboot_mmap_entry*>(mmapAddr + offset);
            if (memBlockEnd == entry->addr)
            {
                memBlockEnd = entry->addr + entry->len;
            }
            else
            {
                break;
            }

            offset += entry->size + sizeof(entry->size);
        }

        // skip map entries below 1 MiB
        if (memBlockEnd > 0x10'0000)
        {
            // skip page frames below 1 MiB
            uint32_t pfStart = memBlockStart;
            if (pfStart < 0x10'0000)
            {
                pfStart = 0x10'0000;
            }

            // find nearest page frame boundary
            if ((pfStart & ~PAGE_BOUNDARY_MASK) != 0)
            {
                pfStart += PAGE_SIZE;
                pfStart &= PAGE_BOUNDARY_MASK;
            }

            // ensure the start is still within the mmap entry bounds
            if (pfStart < memBlockEnd)
            {
                memBlocks[numMemBlocks].startAddr = pfStart;
                memBlocks[numMemBlocks].numPages = (memBlockEnd - pfStart) / PAGE_SIZE;
                ++numMemBlocks;
            }
        }
    }
}

/// @todo Is this function needed?
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
            if ((pfStart & ~PAGE_BOUNDARY_MASK) != 0)
            {
                pfStart += PAGE_SIZE;
                pfStart &= PAGE_BOUNDARY_MASK;
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

/// @todo This function currently makes the simplifying
/// assumption that the data struct will fit in memory
/// right after the kernel
void PageFrameMgr::initDataStruct(const MemBlock* memBlocks, unsigned int numMemBlocks)
{
    // get kernel physical end aligned on a 4-byte boundary
    uint32_t alignedEnd = KERNEL_PHYSICAL_END;
    if ( (alignedEnd & 0x3) != 0)
    {
        alignedEnd = (alignedEnd & ~0x3) + 4;
    }

    blocks = reinterpret_cast<PageFrameBlock*>(alignedEnd + KERNEL_VIRTUAL_BASE);

    // find the last kernel page
    uint32_t pageEnd = alignedEnd;
    if ( (pageEnd & ~PAGE_BOUNDARY_MASK) != 0 )
    {
        pageEnd = (pageEnd & PAGE_BOUNDARY_MASK) + PAGE_SIZE;
    }

    uint32_t blocksEnd = alignedEnd;

    screen << os::Screen::hex
           << os::Screen::setfill('0')
           << "KERNEL_PHYSICAL_END: " << os::Screen::setw(8) << KERNEL_PHYSICAL_END << '\n'
           << "alignedEnd         : " << os::Screen::setw(8) << alignedEnd << '\n'
           << "pageEnd            : " << os::Screen::setw(8) << pageEnd << '\n'
           << os::Screen::dec
           << os::Screen::setfill(' ');

    // init PageFrameBlock structs
    numBlocks = 0;
    for (unsigned int i = 0; i < numMemBlocks; ++i)
    {
        blocksEnd += sizeof(PageFrameBlock);
        if (blocksEnd >= pageEnd)
        {
            mapPage(getKernelPageDirStart(), pageEnd + KERNEL_VIRTUAL_BASE, pageEnd);
            pageEnd += PAGE_SIZE;
        }

        blocks[i].startAddr = memBlocks[i].startAddr;
        blocks[i].numPages = memBlocks[i].numPages;
        ++numBlocks;
    }

    // init isAlloc arrays in each PageFrameBlock struct
    unsigned int totalArraySize = 0;
    uint32_t arrayPtr = blocksEnd;
    for (unsigned int i = 0; i < numBlocks; ++i)
    {
        unsigned int arraySize = blocks[i].numPages / sizeof(uint32_t);

        blocks[i].isAlloc = reinterpret_cast<uint32_t*>(arrayPtr + KERNEL_VIRTUAL_BASE);

        arrayPtr += arraySize;
        totalArraySize += arraySize;
    }

    // calculate number of pages needed for the isAlloc arrays
    unsigned int memNeeded = totalArraySize - (pageEnd - blocksEnd);
    unsigned int pagesNeeded = memNeeded / PAGE_SIZE;
    if (memNeeded % PAGE_SIZE > 0)
    {
        ++pagesNeeded;
    }

    // map pages containing the isAlloc arrays
    for (unsigned int i = 0; i < pagesNeeded; ++i)
    {
        mapPage(getKernelPageDirStart(), pageEnd + KERNEL_VIRTUAL_BASE, pageEnd);
        pageEnd += PAGE_SIZE;
    }
}
