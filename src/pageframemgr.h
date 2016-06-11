/**
 * @brief Page frame manager
 */

#ifndef PAGE_FRAME_MGR_H_
#define PAGE_FRAME_MGR_H_

#include "stdint.h"

// forward declarations
class multiboot_mmap_entry;

/**
 * @brief Page frame manager
 */
class PageFrameMgr
{
public:
    PageFrameMgr(const multiboot_mmap_entry* mmapStart, uint32_t mmapLen);

private:
};

#endif // PAGE_FRAME_MGR_H_
