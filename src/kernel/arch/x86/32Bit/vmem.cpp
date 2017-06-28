#include <pageframemgr.h>
#include <pagetable.h>
#include <paging.h>
#include <screen.h>
#include <vmem.h>

namespace mem {

static uint32_t* _kPageDir = getKernelPageDirStart();

bool isMappedByKernel(uint32_t physAddr, uint32_t& virtAddr)
{
    // check the physical addresses mapped by each valid kernel page...
    // create PageTable for each PDE
    // loop through each PTE, get physAddr, and call isMapped
}

 uint32_t autoMapKernelPageForAddress(uint32_t physAddr, PageFrameMgr* pfMgr)
 {
    // get last kernel PDE in use
    uint16_t lastPDEIdx = mem::lastUsedKernelPDEIndex();
    mem::PageTable pageTable(pfMgr, _kPageDir, lastPDEIdx);
    uint32_t virtAddr = pageTable.mapNextAvailablePageToAddress(physAddr);
    return virtAddr;
 }

}