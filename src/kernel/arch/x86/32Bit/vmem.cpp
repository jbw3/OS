#include <pageframemgr.h>
#include <pagetable.h>
#include <paging.h>
#include <screen.h>
#include <vmem.h>

namespace os {

static uint32_t* _kPageDir = getKernelPageDirStart();

 uint32_t autoMapKernelPageForAddress(uint32_t physAddr, PageFrameMgr* pfMgr)
 {
    // get last kernel PDE in use
    uint16_t lastPDEIdx = pageInfo::lastUsedKernelPDEIndex();
    mem::PageTable pageTable(pfMgr, _kPageDir, lastPDEIdx);
    uint32_t virtAddr = pageTable.mapNextAvailablePageToAddress(physAddr);
    return virtAddr;
 }

}