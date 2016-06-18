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
        /// page in the block is allocated or free
        uint32_t* isAlloc;

        /// pointer to the next page frame block
        PageFrameBlock* nextBlock;
    };

    PageFrameBlock* firstBlock;

    /**
     * @brief get number of page frames from the Multiboot
     * mem map
     * @param mbootInfo pointer to multiboot info
     * @param [out] numPageFrames the number of page frames
     */
    void getMultibootMMapInfo(const multiboot_info* mbootInfo, uint32_t& numPageFrames);
};

#endif // PAGE_FRAME_MGR_H_
