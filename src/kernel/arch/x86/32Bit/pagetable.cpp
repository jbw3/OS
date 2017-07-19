#include "pageframemgr.h"
#include "pagetable.h"
#include "paging.h"
#include "screen.h"
#include "system.h"

namespace mem {

static bool myFlag = false;
static PageTable __ptPageTable;

PageTable::PageTable()
    : _pageDir(nullptr),
    _pageDirIdx(0),
    _pageTable(nullptr)
{
}

PageTable::PageTable(uint16_t pageDirIdx, uint32_t* pageDir/*=nullptr*/)
    : _pageDir(pageDir),
    _pageDirIdx(pageDirIdx)
{
    if (pageDir == nullptr)
    {
        _pageDir = getKernelPageDirStart();
    }
    setPageTablePointer();
}

// private PTPageTable constructor
PageTable::PageTable(uint32_t* ptPageTable, uint16_t pageDirIdx)
    : _pageDir(getKernelPageDirStart()),
    _pageDirIdx(pageDirIdx),
    _pageTable(ptPageTable)
{
}

void PageTable::initKernelPageTablePointer()
{
    // go ahead and set up kernel page table pointer here
    uint32_t* kernelPageDir = getKernelPageDirStart();
    uint32_t kPageDirEntry = kernelPageDir[KERNEL_PAGEDIR_START_IDX];
    uint32_t kPageTableAddrPhys = kPageDirEntry & PAGE_DIR_ADDRESS;
    uint32_t kPageTableAddrVirt = kPageTableAddrPhys + KERNEL_VIRTUAL_BASE;
    __pageTablePtrs[0].kPageDirIdx = KERNEL_PAGEDIR_START_IDX;
    __pageTablePtrs[0].pageTable = (uint32_t*)kPageTableAddrVirt;
}

void PageTable::initPTPageTable(uint32_t* ptPageTable, uint16_t pageDirIdx)
{
    __ptPageTable = PageTable(ptPageTable, pageDirIdx);
    __ptPageTable.clearPageTable();      // clear all present bits

    // set up PTPT in page table pointer array
    auto idx = __ptPageTable.ptArrayIndex();
    __pageTablePtrs[idx].kPageDirIdx = pageDirIdx;
    __pageTablePtrs[idx].pageTable = ptPageTable;
}

PageTable* PageTable::getPTPageTable()
{
    return &__ptPageTable;
}

uint32_t* PageTable::getPointer()
{
    return _pageTable;
}

uint32_t* PageTable::getPageDirPointer()
{
    return _pageDir;
}

uint16_t PageTable::getPageDirIndex()
{
    return _pageDirIdx;
}

bool PageTable::isFull()
{
    // cls: start at end for speed - loop will execute 1x
    // for page tables filled from front to back

    for (int16_t i = PAGE_TABLE_NUM_ENTRIES-1; i >= 0; i--)
    {
        if (!(_pageTable[i] & PAGE_TABLE_PRESENT))
        {
            return false;   // found an available entry
        }
    }
    return true;    // each entry was used
}

bool PageTable::isEmpty()
{
    for (uint16_t i = 0; i < PAGE_TABLE_NUM_ENTRIES; i++)
    {
        if (_pageTable[i] & PAGE_TABLE_PRESENT)
        {
            return false;   // found an active entry
        }
    }
    return true;    // each entry was inactive
}

void PageTable::clearPageTable()
{
    screen << "entered init\n";
    screen << "_pageTable: " << _pageTable << "\n";
    screen << "kpd: " << (uint32_t)getKernelPageDirStart() << "\n";
    screen << "_pageTable[0]: " << _pageTable[0] << "\n";

    for (int i = 0; i < PAGE_TABLE_NUM_ENTRIES; i++)
    {
        _pageTable[i] &= ~PAGE_TABLE_PRESENT;
    }
}

void PageTable::init()
{
    screen << "PT init: _pageTable=0x" << _pageTable << "\n";
    // get page frame address
    uint32_t pageFrameAddr = _pageDir[_pageDirIdx] & PAGE_DIR_ADDRESS;
    // map this pageTable in the PTPT
    uint32_t pageTableVirtAddr = __ptPageTable.mapNextAvailablePageToAddress(pageFrameAddr);
    // set _pageTable*
    _pageTable = (uint32_t*)pageTableVirtAddr;
    screen << "PT init: _pageTable=0x" << _pageTable << "\n";

    // save page table info in page table pointer array
    auto idx = ptArrayIndex();
    __pageTablePtrs[idx].kPageDirIdx = _pageDirIdx;
    __pageTablePtrs[idx].pageTable = _pageTable;

    // now that the page table itself is mapped, let's clear it out...
    clearPageTable();
    screen << "completed init!\n";
}

uint16_t PageTable::nextAvailablePage()
{
    screen << "_pageTable " << (uint32_t)_pageTable << "\n";
    screen << "_pageTable[0]" << _pageTable[0] << "\n";
    for (uint16_t i = 0; i < PAGE_TABLE_NUM_ENTRIES; i++)
    {
        if (!(_pageTable[i] & PAGE_TABLE_PRESENT))
        {
            return i;   // this entry is available
        }
    }
    return PAGE_TABLE_NUM_ENTRIES;  // invalid index
}

uint32_t* PageTable::mapPage(uint16_t pageIdx, uint32_t physAddr)
{
    if (_pageTable[pageIdx] & PAGE_TABLE_PRESENT)
    {
        return nullptr;     // this page is already mapped
    }
    else
    {
        uint32_t pte = _pageTable[pageIdx];
        pte &= ~PAGE_TABLE_ADDRESS;     // clear previous address
        pte |= (physAddr & PAGE_TABLE_ADDRESS); // new address
        pte |= PAGE_TABLE_READ_WRITE | PAGE_TABLE_PRESENT;  // set R/W and present
        _pageTable[pageIdx] = pte;

        // return virt page address + offset from phys addr
        return (uint32_t*)(virtAddressOfPage(pageIdx) | (physAddr & 0xFFF));
    }
}

uint32_t PageTable::mapNextAvailablePageToAddress(uint32_t physAddr)
{
    auto virtPointer = mapPage(nextAvailablePage(), physAddr);
    if (virtPointer != nullptr)
    {
        screen << "mapped page!\n";
        PageFrameMgr::get()->reservePageFrame(physAddr);    // mark as reserved for page frame manager
        return (uint32_t) virtPointer;  // TODO: return uint32_t* here...
    }
    else
    {
        return (uint32_t)virtPointer;
    }
}

uint32_t PageTable::virtAddressOfPage(uint16_t pageIdx)
{
    uint32_t address = (_pageDirIdx & 0x3FF) << 22; // 10 bits of page dir index
    address |= (pageIdx & 0x3FF) << 12;             // 10 bits of page table index
    return address;
}

uint32_t PageTable::physAddressOfPageFrame(uint16_t pageIdx)
{
    return _pageTable[pageIdx] & PAGE_TABLE_ADDRESS;
}

bool PageTable::isMapped(uint32_t physAddr, uint32_t& virtAddr)
{
    for (uint16_t i = 0; i < PAGE_TABLE_NUM_ENTRIES; i++)
    {
        //screen << "_pageTable value: 0x" << (uint32_t)_pageTable << "\n";
        //screen << "accessed pageTable[i]\n";
        if (_pageTable[i] & PAGE_TABLE_PRESENT)
        {
            // get physical bounds of page frame
            uint32_t pageFrameAddress = physAddressOfPageFrame(i);
            uint32_t maxPageFrameAddr = pageFrameAddress + 0xFFF;

            // does the physical address fall within this page frame?
            if (physAddr >= pageFrameAddress && physAddr <= maxPageFrameAddr)
            {
                // set the corresponding virtual address
                auto offset = physAddr & 0xFFF;
                auto virtBase = (_pageDirIdx & 0x3FF) << 22;    // 10 bits of page dir index
                virtBase |= (i & 0x3FF) << 12;                  // 10 bits of page table index
                virtAddr = virtBase | offset;
                return true;
            }
        }
    }
    return false;   // no mappings found
}

int PageTable::ptArrayIndex()
{
    return _pageDirIdx - KERNEL_PAGEDIR_START_IDX;
}

void PageTable::setPageTablePointer()
{
    _pageTable = __pageTablePtrs[ptArrayIndex()].pageTable;
}

}