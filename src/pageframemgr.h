/**
 * @brief Page frame manager
 */

#ifndef PAGE_FRAME_MGR_H_
#define PAGE_FRAME_MGR_H_

#include "stdint.h"

// forward declarations
struct multiboot_info;

/**
 * @brief Page frame manager
 */
class PageFrameMgr
{
public:
    PageFrameMgr(const multiboot_info* mbootInfo);

private:
    struct MemBlock
    {
        uint32_t startAddr;
        uint32_t numPages;
    };

    /**
     * @brief A block of contiguous page frames in memory
     */
    struct PageFrameBlock
    {
        /// physical address of the first page in the block
        uint32_t startAddr;

        /// number of pages in block
        uint32_t numPages;

        /// pointer to array of bits that indicate whether each
        /// page in the block is free or allocated (0 = free,
        /// 1 = allocated)
        uint32_t* isAlloc;
    };

    PageFrameBlock* blocks;
    unsigned int numBlocks;

    void initMemBlocks(const multiboot_info* mbootInfo, MemBlock* memBlocks, unsigned int memBlocksSize, unsigned int& numMemBlocks);

    /**
     * @brief get number of page frames from the Multiboot
     * mem map
     * @param mbootInfo pointer to multiboot info
     * @param [out] numPageFrames the number of page frames
     */
    void getMultibootMMapInfo(const multiboot_info* mbootInfo, uint32_t& numPageFrames);

    /**
     * @brief Allocate and initialize the page frame data structure
     */
    void initDataStruct(const MemBlock* memBlocks, unsigned int numMemBlocks);
};

#endif // PAGE_FRAME_MGR_H_
