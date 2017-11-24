#include "pagetreex86_32.h"

PageTreeX86_32::PageTreeX86_32(uintptr_t pageDirAddr)
{
    pageDir = reinterpret_cast<uintptr_t*>(pageDirAddr);
}

bool PageTreeX86_32::map(uintptr_t virtualAddr, uintptr_t physicalAddr)
{
}

bool PageTreeX86_32::mapOnOrAfter(uintptr_t startAddr, uintptr_t& virtualAddr, uintptr_t physicalAddr)
{
}

bool PageTreeX86_32::mapOnOrBefore(uintptr_t startAddr, uintptr_t& virtualAddr, uintptr_t physicalAddr)
{
}

void PageTreeX86_32::unmap(uintptr_t virtualAddr)
{
}
