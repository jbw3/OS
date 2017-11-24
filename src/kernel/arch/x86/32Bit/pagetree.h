#ifndef PAGE_TREE_H_
#define PAGE_TREE_H_

#include <stdint.h>

/**
 * @brief Abstract base class for a page tree structure.
 */
class PageTree
{
public:
    /**
     * @brief Map a page at a specified virtual address.
     */
    virtual bool map(uintptr_t physicalAddr, uintptr_t virtualAddr) = 0;

    virtual bool mapOnOrAfter(uintptr_t startAddr, uintptr_t physicalAddr, uintptr_t& virtualAddr) = 0;

    virtual bool mapOnOrBefore(uintptr_t startAddr, uintptr_t physicalAddr, uintptr_t& virtualAddr) = 0;

    virtual void unmap(uintptr_t virtualAddr) = 0;
};

#endif // PAGE_TREE_H_
