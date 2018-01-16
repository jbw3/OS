#ifndef PAGE_TREE_H_
#define PAGE_TREE_H_

#include <stdint.h>

/**
 * @brief Abstract base class for a page tree structure.
 */
class PageTree
{
protected:
    static PageTree* kernelPageTree;

public:
    enum EFlags
    {
        /// User processes have access to the page.
        eUser = 0x1,

        /// The page can be read and written to.
        eReadWrite = 0x2,
    };

    typedef unsigned long Entry;

    static PageTree& getKernelPageTree()
    {
        return *kernelPageTree;
    }

    /**
     * @brief Map a page at a specified virtual address.
     */
    virtual bool map(uintptr_t virtualAddr, uintptr_t physicalAddr, unsigned int flags = 0) = 0;

    virtual bool mapOnOrAfter(uintptr_t startAddr, uintptr_t& virtualAddr, uintptr_t physicalAddr, unsigned int flags = 0) = 0;

    virtual bool mapOnOrBefore(uintptr_t startAddr, uintptr_t& virtualAddr, uintptr_t physicalAddr, unsigned int flags = 0) = 0;

    virtual void unmap(uintptr_t virtualAddr) = 0;

    /**
     * @brief Get the physical address mapped to the given virtual address.
     */
    virtual bool getPhysicalAddress(uintptr_t virtualAddr, uintptr_t& physicalAddr) const = 0;
};

#endif // PAGE_TREE_H_
