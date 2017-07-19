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
        screen << physAddr << " not mapped by kernel\n";
        virtAddr = autoMapKernelPageForAddress(physAddr);
        screen << "done automapping " << physAddr << " to " << virtAddr << "\n";
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
            screen << "page dir " << i << " present\n";
            PageTable pt(i);
            if (pt.isMapped(physAddr, virtAddr))
            {
                screen << "mapped\n";
                return true;    // physAddr is mapped, virtAddr has been set
            }
            screen << "not mapped\n";
        }
    }
    return false;   // did not find a mapping
}

 uint32_t autoMapKernelPageForAddress(uint32_t physAddr)
 {
    // get last kernel PDE in use
    uint16_t lastPDEIdx = mem::lastUsedKernelPDEIndex();
    screen << "PDE Idx: 0x" << lastPDEIdx << "\n";
    mem::PageTable pageTable(lastPDEIdx);
    if (!pageTable.isFull())
    {
        screen << "is not full\n";
        return pageTable.mapNextAvailablePageToAddress(physAddr);
    }
    else
    {
        auto nextPDEIndex = mem::nextAvailableKernelPDEIndex();
        screen << "is full. Next available PDE index for a page table is: 0x" << nextPDEIndex << "\n";

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

        screen << "alloc page frame & set PDE\n";

        mem::PageTable nextPageTable(nextPDEIndex);
        screen << "next PT\n";
        nextPageTable.init();
        screen << "init page table\n";

        if (pageFramePhys == 0x114000)
        {
            screen << "\n";
            screen << "set newPDE to 0x" << (uint32_t)((nextPDEIndex) << 22) << "\n";
            screen << "\n";

            // the problem is, I need a way to get TO the page table (@ phys addr 0x114000)
            // through virtual memory...

            screen << "allocated 0x114000\n";
            //uint32_t* entries = (uint32_t*)(pageFramePhys);
            //screen << "114000 page table entries: 0x" <<
        }

        return nextPageTable.mapNextAvailablePageToAddress(physAddr);
    }
 }

}