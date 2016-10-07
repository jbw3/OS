/**
 * @brief Page frame manager
 */

#ifndef PAGE_FRAME_MGR_H_
#define PAGE_FRAME_MGR_H_

#include "stddef.h"
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

    /**
     * @brief Allocate a page frame
     * @return the physical address of the allocated memory or
     * zero if no memory could be allocated
     */
    uintptr_t allocPageFrame();

    /**
     * @brief Free a page frame
     */
    void freePageFrame(uintptr_t addr);

    // ------ Debugging ------

    bool isPageFrameAlloc(uintptr_t addr) const;

    void printBlocks() const;

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
        uintptr_t startAddr;

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
     * @details This is for debugging
     * @param mbootInfo pointer to multiboot info
     * @param [out] numPageFrames the number of page frames
     */
    void getMultibootMMapInfo(const multiboot_info* mbootInfo, uint32_t& numPageFrames) const;

    /**
     * @brief Allocate and initialize the page frame data structure
     */
    void initDataStruct(const MemBlock* memBlocks, unsigned int numMemBlocks);

    /**
     * @brief Mark the page frames in use by the kernel
     */
    void markKernel();

    bool findPageFrame(uintptr_t addr, unsigned int& blockIdx, unsigned int& allocIdx, uint32_t& bitMask) const;

    unsigned int getIsAllocSize(const PageFrameBlock& pfBlock) const;
};

#endif // PAGE_FRAME_MGR_H_
