#include "pageframemgr.h"
#include "pagetable.h"
#include "paging.h"
#include "screen.h"
#include "vmem.h"

namespace mem {

static uint32_t* _kPageDir = getKernelPageDirStart();

uint32_t toVirtualKernelAddr(uint32_t physAddr)
{
    uint32_t virtAddr = 0;

    if (!isMappedByKernel(physAddr, virtAddr))
    {
        virtAddr = autoMapKernelPageForAddress(physAddr);
    }
    return virtAddr;
}

bool isMappedByKernel(uint32_t physAddr, uint32_t& virtAddr)
{
    // check the physical addresses mapped by each valid kernel page...
    for (uint16_t i = KERNEL_PAGEDIR_START_IDX; i < PAGE_DIR_NUM_ENTRIES; i++)
    {
        if (_kPageDir[i] & PAGE_DIR_PRESENT)
        {
            PageTable pt(i);
            if (pt.isMapped(physAddr, virtAddr))
            {
                return true;    // physAddr is mapped, virtAddr has been set
            }
        }
    }
    return false;   // did not find a mapping
}

 uint32_t autoMapKernelPageForAddress(uint32_t physAddr)
 {
    // get last kernel PDE in use
    uint16_t lastPDEIdx = mem::lastUsedKernelPDEIndex();
    mem::PageTable pageTable(lastPDEIdx);
    if (!pageTable.isFull())
    {
        return pageTable.mapNextAvailablePageToAddress(physAddr);
    }
    else
    {
        auto nextPDEIndex = mem::nextAvailableKernelPDEIndex();

        // todo: move to PageDirectory class...

        // where do we put new page tables? need to inform PFM?
        auto pageFramePhys = PageFrameMgr::get()->allocPageFrame();
        //screen << "pageFramePhys 0x" << pageFramePhys << "\n";
        uint32_t pde = _kPageDir[nextPDEIndex];
        pde &= ~PAGE_DIR_ADDRESS;   // clear previous addresss
        pde |= (PAGE_DIR_ADDRESS & pageFramePhys);
        pde |= PAGE_DIR_READ_WRITE | PAGE_DIR_PRESENT;
        // uint32_t newPDE =   (PAGE_DIR_ADDRESS & pageFramePhys) |
        //                     (PAGE_DIR_READ_WRITE) |
        //                     (PAGE_DIR_PRESENT);
        _kPageDir[nextPDEIndex] = pde;

        // set up new page table
        mem::PageTable nextPageTable(nextPDEIndex);
        nextPageTable.init();   // clear contents and map in the PTPT

        return nextPageTable.mapNextAvailablePageToAddress(physAddr);
    }
 }

}