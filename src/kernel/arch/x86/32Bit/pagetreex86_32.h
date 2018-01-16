#ifndef PAGE_TREE_X86_32_H_
#define PAGE_TREE_X86_32_H_

#include "memoryunits.h"
#include "pagetree.h"

class PageTreeX86_32 : public PageTree
{
public:
    PageTreeX86_32(uintptr_t pageDirAddr, bool isKernel);

    bool map(uintptr_t virtualAddr, uintptr_t physicalAddr, unsigned int flags = 0) override;

    bool mapOnOrAfter(uintptr_t startAddr, uintptr_t& virtualAddr, uintptr_t physicalAddr, unsigned int flags = 0) override;

    bool mapOnOrBefore(uintptr_t startAddr, uintptr_t& virtualAddr, uintptr_t physicalAddr, unsigned int flags = 0) override;

    void unmap(uintptr_t virtualAddr) override;

    bool getPhysicalAddress(uintptr_t virtualAddr, uintptr_t& physicalAddr) const override;

private:
    /// The starting virtual address of the kernel's page tables
    static Entry* const KERNEL_PAGE_TABLES;

    /// The starting virtual address of each process's page tables
    static Entry* const PROCESS_PAGE_TABLES;

    Entry* pageDir;
    Entry* const pageTables;

    /**
     * @brief Calculate the virtual page table address.
     */
    Entry* getPageTable(int pageDirIdx) const;
};

#endif // PAGE_TREE_X86_32_H_
