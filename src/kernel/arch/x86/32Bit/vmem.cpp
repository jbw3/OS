#include <pageframemgr.h>
#include <pagetable.h>
#include <paging.h>
#include <screen.h>
#include <vmem.h>

namespace mem {

static uint32_t* _kPageDir = getKernelPageDirStart();
static PageFrameMgr* _pfMgr = nullptr;

void setPageFrameMgr(PageFrameMgr* pfMgr)
{
    _pfMgr = pfMgr;
}

uint32_t toVirtualKernelAddr(uint32_t physAddr)
{
    uint32_t virtAddr = 0;
    if (!isMappedByKernel(physAddr, virtAddr))
    {
        virtAddr = autoMapKernelPageForAddress(physAddr, _pfMgr);
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
            PageTable pt(_pfMgr, _kPageDir, i);
            if (pt.isMapped(physAddr, virtAddr))
            {
                return true;    // physAddr is mapped, virtAddr has been set
            }
        }
    }
    return false;   // did not find a mapping
}

 uint32_t autoMapKernelPageForAddress(uint32_t physAddr, PageFrameMgr* pfMgr)
 {
    // get last kernel PDE in use
    uint16_t lastPDEIdx = mem::lastUsedKernelPDEIndex();
    mem::PageTable pageTable(pfMgr, _kPageDir, lastPDEIdx);
    if (!pageTable.isFull())
    {
        return pageTable.mapNextAvailablePageToAddress(physAddr);
    }
    else
    {
        // todo: move to PageDirectory class...

        // where do we put new page tables? need to inform PFM?
        auto pageFramePhys = pfMgr->allocPageFrame();
        //screen << "pageFramePhys 0x" << pageFramePhys << "\n";
        uint32_t newPDE =   (PAGE_DIR_ADDRESS & pageFramePhys) |
                            (PAGE_DIR_READ_WRITE) |
                            (PAGE_DIR_PRESENT);
        _kPageDir[lastPDEIdx+1] = newPDE;

        mem::PageTable nextPageTable(pfMgr, _kPageDir, lastPDEIdx+1);
        nextPageTable.initPageTable();

        if (pageFramePhys == 0x114000)
        {
            screen << "\n";
            screen << "set newPDE to 0x" << (uint32_t)((lastPDEIdx+1) << 22) << "\n";
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