#ifndef PAGE_TREE_X86_32_H_
#define PAGE_TREE_X86_32_H_

#include "pagetree.h"

class PageTreeX86_32 : public PageTree
{
public:
    PageTreeX86_32(uintptr_t pageDirAddr);

    bool map(uintptr_t virtualAddr, uintptr_t physicalAddr) override;

    bool mapOnOrAfter(uintptr_t startAddr, uintptr_t& virtualAddr, uintptr_t physicalAddr) override;

    bool mapOnOrBefore(uintptr_t startAddr, uintptr_t& virtualAddr, uintptr_t physicalAddr) override;

    void unmap(uintptr_t virtualAddr) override;

private:
    uintptr_t* pageDir;
};

#endif // PAGE_TREE_X86_32_H_
