#include "gdt.h"
#include "multiboot.h"
#include "pageframemgr.h"
#include "processmgr.h"
#include "screen.h"
#include "string.h"
#include "system.h"

const uintptr_t ProcessMgr::ProcessInfo::KERNEL_STACK_PAGE = KERNEL_VIRTUAL_BASE - PAGE_SIZE;
const uintptr_t ProcessMgr::ProcessInfo::USER_STACK_PAGE = ProcessMgr::ProcessInfo::KERNEL_STACK_PAGE - PAGE_SIZE;

// put the ProcessInfo pointer at the base of the kernel stack
ProcessMgr::ProcessInfo** ProcessMgr::ProcessInfo::PROCESS_INFO = reinterpret_cast<ProcessMgr::ProcessInfo**>(ProcessMgr::ProcessInfo::KERNEL_STACK_PAGE + PAGE_SIZE - 4);

// the kernel stack starts right after the ProcessInfo pointer
const uintptr_t ProcessMgr::ProcessInfo::KERNEL_STACK_START = reinterpret_cast<uintptr_t>(ProcessMgr::ProcessInfo::PROCESS_INFO) - sizeof(ProcessMgr::ProcessInfo::PROCESS_INFO);

ProcessMgr::ProcessInfo::ProcessInfo()
{
    reset();
}

void ProcessMgr::ProcessInfo::reset()
{
    id = 0;
    pageDir = {0, 0};
    kernelPageTable = {0, 0};
    lowerPageTable = {0, 0};
    upperPageTable = {0, 0};
    numPages = 0;
}

void ProcessMgr::ProcessInfo::addPage(const PageFrameInfo& info)
{
    pages[numPages++] = info;
}

ProcessMgr::ProcessInfo::PageFrameInfo ProcessMgr::ProcessInfo::getPage(int i) const
{
    return pages[i];
}

int ProcessMgr::ProcessInfo::getNumPages() const
{
    return numPages;
}

ProcessMgr::ProcessMgr() :
    pageFrameMgr(nullptr)
{
}

void ProcessMgr::setPageFrameMgr(PageFrameMgr* pageFrameMgrPtr)
{
    pageFrameMgr = pageFrameMgrPtr;
}

void ProcessMgr::createProcess(const multiboot_mod_list* module)
{
    bool ok = true;

    // find an entry in the process info table
    ProcessInfo* newProcInfo = nullptr;
    ok = getNewProcInfo(newProcInfo);

    if (ok)
    {
        ok = initPaging(newProcInfo, getKernelPageTableStart());
    }

    if (ok)
    {
        // copy kernel page table
        copyKernelPageTable(newProcInfo, getKernelPageTableStart());

        // create process page tables
        createProcessPageTables(newProcInfo);

        // unmap process pages from kernel page table
        unmapPages(newProcInfo, getKernelPageTableStart());

        // switch to process's page directory
        setPageDirectory(newProcInfo->pageDir.physicalAddr);

        // copy the program and set up the stack
        ok = setUpProgram(module, newProcInfo);
    }

    if (ok)
    {
        // allocate a process ID
        newProcInfo->id = getNewId();

        // set the ProcessInfo pointer
        *ProcessInfo::PROCESS_INFO = newProcInfo;

        // set the kernel stack for the process
        setKernelStack(ProcessInfo::KERNEL_STACK_START);

        // switch to user mode and run process
        switchToUserMode(ProcessInfo::USER_STACK_PAGE + PAGE_SIZE - 4, &kernelStack);

        // --- we get back here when the process has exited ---

        // switch to kernel's page directory
        uintptr_t kernelPageDirPhyAddr = reinterpret_cast<uintptr_t>(getKernelPageDirStart()) - KERNEL_VIRTUAL_BASE;
        setPageDirectory(kernelPageDirPhyAddr);

        // enable interrupts
        setInt();

        cleanUpProcess(newProcInfo);
    }
    else
    {
        // if we get here, something went wrong and we need to
        // clean things up

        // switch back to kernel's page directory
        uintptr_t kernelPageDirPhyAddr = reinterpret_cast<uintptr_t>(getKernelPageDirStart()) - KERNEL_VIRTUAL_BASE;
        setPageDirectory(kernelPageDirPhyAddr);

        cleanUpProcess(newProcInfo);
    }
}

bool ProcessMgr::forkCurrentProcess()
{
    bool ok = true;

    // find an entry in the process info table
    ProcessInfo* newProcInfo = nullptr;
    ok = getNewProcInfo(newProcInfo);

    uintptr_t* currentKernelPageTable = reinterpret_cast<uintptr_t*>(getCurrentProcessInfo()->kernelPageTable.virtualAddr);

    if (ok)
    {
        ok = initPaging(newProcInfo, currentKernelPageTable);
    }

    if (ok)
    {
        // copy kernel page table
        copyKernelPageTable(newProcInfo, currentKernelPageTable);

        // copy process page tables
        copyProcessPageTables(newProcInfo, getCurrentProcessInfo());

        // copy process's pages
        ok = copyProcessPages(newProcInfo, getCurrentProcessInfo());
    }

    if (ok)
    {
        // unmap parent process pages from child process page table
        unmapPages(getCurrentProcessInfo(), reinterpret_cast<uintptr_t*>(newProcInfo->kernelPageTable.virtualAddr));

        // unmap child process pages from parent process page table
        unmapPages(newProcInfo, currentKernelPageTable);

        // allocate a process ID
        newProcInfo->id = getNewId();

        // set the ProcessInfo pointer
        *ProcessInfo::PROCESS_INFO = newProcInfo;

        // set the kernel stack for the process
        setKernelStack(ProcessInfo::KERNEL_STACK_START);

        // switch to user mode, set page dir, and run process
        switchToUserModeAndSetPageDir(ProcessInfo::USER_STACK_PAGE + PAGE_SIZE - 4,
                                      &getCurrentProcessInfo()->stack,
                                      newProcInfo->pageDir.physicalAddr);
    }
    else
    {
        // if we get here, something went wrong and we need to
        // clean things up

        cleanUpProcess(newProcInfo);
    }

    return ok;
}

void ProcessMgr::exitCurrentProcess()
{
    // switch back to kernel stack
    switchToProcessStack(kernelStack);
}

ProcessMgr::ProcessInfo* ProcessMgr::getCurrentProcessInfo()
{
    return *ProcessInfo::PROCESS_INFO;
}

bool ProcessMgr::getNewProcInfo(ProcessInfo*& procInfo)
{
    procInfo = nullptr;
    for (int i = 0; i < MAX_NUM_PROCESSES; ++i)
    {
        if (processInfo[i].id == 0)
        {
            procInfo = &processInfo[i];
            break;
        }
    }

    if (procInfo == nullptr)
    {
        logError("The maximum number of processes has already been created.");
        return false;
    }

    return true;
}

bool ProcessMgr::initPaging(ProcessInfo* procInfo, uintptr_t* pageTable)
{
    // Allocate pages for page directory, kernel page table, lower page
    // table (for code and data), and upper page table (for stacks).
    constexpr int NUM_PAGES = 4;
    for (int i = 0; i < NUM_PAGES; ++i)
    {
        // get page frames for the process's page dir and page tables
        // (these are physical addresses)
        uintptr_t phyAddr = pageFrameMgr->allocPageFrame();

        // log an error if we could not get a page frame
        if (phyAddr == 0)
        {
            logError("Could not allocate page frame.");
            return false;
        }

        // We need to map the page dir and table to modify them.
        uintptr_t virAddr = 0;
        if (!mapPage((KERNEL_VIRTUAL_BASE >> 22), pageTable, virAddr, phyAddr))
        {
            logError("Could not map page.");
            return false;
        }

        switch (i)
        {
        case 0:
            procInfo->pageDir = {virAddr, phyAddr};
            break;
        case 1:
            procInfo->kernelPageTable = {virAddr, phyAddr};
            break;
        case 2:
            procInfo->lowerPageTable = {virAddr, phyAddr};
            break;
        case 3:
            procInfo->upperPageTable = {virAddr, phyAddr};
            break;
        default:
            PANIC("We shouldn't get here.");
            return false;
            break;
        }
    }

    return true;
}

void ProcessMgr::unmapPages(ProcessInfo* procInfo, uintptr_t* pageTable)
{
    unmapPage(pageTable, procInfo->pageDir.virtualAddr);
    unmapPage(pageTable, procInfo->kernelPageTable.virtualAddr);
    unmapPage(pageTable, procInfo->lowerPageTable.virtualAddr);
    unmapPage(pageTable, procInfo->upperPageTable.virtualAddr);
}

void ProcessMgr::copyKernelPageTable(ProcessInfo* dstProc, uintptr_t* srcKernelPageTable)
{
    uintptr_t* pageDir = reinterpret_cast<uintptr_t*>(dstProc->pageDir.virtualAddr);
    uintptr_t* kernelPageTable = reinterpret_cast<uintptr_t*>(dstProc->kernelPageTable.virtualAddr);

    // copy kernel page table
    memcpy(kernelPageTable, srcKernelPageTable, PAGE_SIZE);

    // map kernel page table in page directory
    memset(pageDir, 0, PAGE_SIZE);
    int kernelIdx = KERNEL_VIRTUAL_BASE >> 22;
    mapPageTable(pageDir, dstProc->kernelPageTable.physicalAddr, kernelIdx);
}

void ProcessMgr::createProcessPageTables(ProcessInfo* newProcInfo)
{
    uintptr_t* pageDir = reinterpret_cast<uintptr_t*>(newProcInfo->pageDir.virtualAddr);
    uintptr_t* lowerPageTable = reinterpret_cast<uintptr_t*>(newProcInfo->lowerPageTable.virtualAddr);
    uintptr_t* upperPageTable = reinterpret_cast<uintptr_t*>(newProcInfo->upperPageTable.virtualAddr);

    // clear lower and upper memory page tables
    memset(lowerPageTable, 0, PAGE_SIZE);
    memset(upperPageTable, 0, PAGE_SIZE);

    // map lower page table in page directory
    mapPageTable(pageDir, newProcInfo->lowerPageTable.physicalAddr, 0, true);

    // map upper page table in page directory right before kernel page table
    int upperIdx = (KERNEL_VIRTUAL_BASE - PAGE_SIZE) >> 22;
    mapPageTable(pageDir, newProcInfo->upperPageTable.physicalAddr, upperIdx, true);
}

void ProcessMgr::copyProcessPageTables(ProcessInfo* dstProc, ProcessInfo* srcProc)
{
    uintptr_t* dstPageDir = reinterpret_cast<uintptr_t*>(dstProc->pageDir.virtualAddr);
    uintptr_t* dstLowerPageTable = reinterpret_cast<uintptr_t*>(dstProc->lowerPageTable.virtualAddr);
    uintptr_t* dstUpperPageTable = reinterpret_cast<uintptr_t*>(dstProc->upperPageTable.virtualAddr);
    uintptr_t* srcLowerPageTable = reinterpret_cast<uintptr_t*>(srcProc->lowerPageTable.virtualAddr);
    uintptr_t* srcUpperPageTable = reinterpret_cast<uintptr_t*>(srcProc->upperPageTable.virtualAddr);

    // copy page tables
    memcpy(dstLowerPageTable, srcLowerPageTable, PAGE_SIZE);
    memcpy(dstUpperPageTable, srcUpperPageTable, PAGE_SIZE);

    // map lower page table in page directory
    mapPageTable(dstPageDir, dstProc->lowerPageTable.physicalAddr, 0, true);

    // map upper page table in page directory right before kernel page table
    int upperIdx = (KERNEL_VIRTUAL_BASE - PAGE_SIZE) >> 22;
    mapPageTable(dstPageDir, dstProc->upperPageTable.physicalAddr, upperIdx, true);
}

bool ProcessMgr::setUpProgram(const multiboot_mod_list* module, ProcessInfo* newProcInfo)
{
    uintptr_t* lowerPageTable = reinterpret_cast<uintptr_t*>(newProcInfo->lowerPageTable.virtualAddr);
    uintptr_t* upperPageTable = reinterpret_cast<uintptr_t*>(newProcInfo->upperPageTable.virtualAddr);

    // allocate and map pages for code
    uintptr_t virAddr = ProcessInfo::CODE_VIRTUAL_START;
    for (unsigned int ptr = module->mod_start; ptr < module->mod_end; ptr += PAGE_SIZE)
    {
        uintptr_t phyAddr = pageFrameMgr->allocPageFrame();
        if (phyAddr == 0)
        {
            logError("Could not allocate page frame.");
            return false;
        }

        newProcInfo->addPage({virAddr, phyAddr});
        mapPage(lowerPageTable, virAddr, phyAddr, true);

        virAddr += PAGE_SIZE;
    }

    // copy process's code
    size_t codeSize = module->mod_end - module->mod_start;
    memcpy(reinterpret_cast<void*>(ProcessInfo::CODE_VIRTUAL_START),
           reinterpret_cast<const void*>(module->mod_start + KERNEL_VIRTUAL_BASE),
           codeSize);

    // allocate and map a page for the kernel stack
    uintptr_t kernelStackPhyAddr = pageFrameMgr->allocPageFrame();
    if (kernelStackPhyAddr == 0)
    {
        logError("Could not allocate a page frame for the kernel stack.");
        return false;
    }
    newProcInfo->addPage({ProcessInfo::KERNEL_STACK_PAGE, kernelStackPhyAddr});
    mapPage(upperPageTable, ProcessInfo::KERNEL_STACK_PAGE, kernelStackPhyAddr);

    // allocate and map a page for the user stack
    uintptr_t userStackPhyAddr = pageFrameMgr->allocPageFrame();
    if (userStackPhyAddr == 0)
    {
        logError("Could not allocate a page frame for the user stack.");
        return false;
    }
    newProcInfo->addPage({ProcessInfo::USER_STACK_PAGE, userStackPhyAddr});
    mapPage(upperPageTable, ProcessInfo::USER_STACK_PAGE, userStackPhyAddr, true);

    return true;
}

bool ProcessMgr::copyProcessPages(ProcessInfo* dstProc, ProcessInfo* srcProc)
{
    uintptr_t* srcKernelPageTable = reinterpret_cast<uintptr_t*>(srcProc->kernelPageTable.virtualAddr);

    for (int i = 0; i < srcProc->getNumPages(); ++i)
    {
        ProcessInfo::PageFrameInfo srcPageInfo = srcProc->getPage(i);

        // allocate a page
        uintptr_t phyAddr = pageFrameMgr->allocPageFrame();
        if (phyAddr == 0)
        {
            logError("Could not allocate a page frame for the new process.");
            return false;
        }
        uintptr_t virAddr = srcPageInfo.virtualAddr;
        dstProc->addPage({virAddr, phyAddr});

        // find page table to map page into
        uintptr_t* dstPageTable = nullptr;
        switch (virAddr >> 22)
        {
        case 0:
            dstPageTable = reinterpret_cast<uintptr_t*>(dstProc->lowerPageTable.virtualAddr);
            break;
        case 767:
            dstPageTable = reinterpret_cast<uintptr_t*>(dstProc->upperPageTable.virtualAddr);
            break;
        case 768:
            dstPageTable = reinterpret_cast<uintptr_t*>(dstProc->kernelPageTable.virtualAddr);
            break;
        default:
            PANIC("We should not get here!");
            return false;
            break;
        }

        // map the page
        mapPage(dstPageTable, virAddr, phyAddr, true);

        // temporarily map the destination page in the current process's page
        // table so we can copy to it
        uintptr_t tempAddr = 0;
        bool ok = mapPage((KERNEL_VIRTUAL_BASE >> 22), srcKernelPageTable, tempAddr, phyAddr);
        if (!ok)
        {
            logError("Could not map temporary page.");
            return false;
        }

        // copy the page
        memcpy(reinterpret_cast<void*>(tempAddr), reinterpret_cast<const void*>(virAddr), PAGE_SIZE);

        // unmap the destination page from the current process's page table
        unmapPage(srcKernelPageTable, tempAddr);
    }

    return true;
}

pid_t ProcessMgr::getNewId()
{
    static pid_t nextId = 1;

    bool invalidId = false;
    do
    {
        invalidId = false;

        // make sure the ID is not 0
        if (nextId == 0)
        {
            ++nextId;
        }

        // make sure no other process has the same ID
        for (int i = 0; i < MAX_NUM_PROCESSES; ++i)
        {
            if (processInfo[i].id == nextId)
            {
                ++nextId;
                invalidId = true;
                break;
            }
        }
    } while (invalidId);

    return nextId++;
}

ProcessMgr::ProcessInfo* ProcessMgr::findProcess(pid_t id)
{
    for (int i = 0; i < MAX_NUM_PROCESSES; ++i)
    {
        if (processInfo[i].id == id)
        {
            return &processInfo[i];
        }
    }

    return nullptr;
}

void ProcessMgr::cleanUpProcess(ProcessInfo* procInfo)
{
    // free paging structures
    pageFrameMgr->freePageFrame(procInfo->pageDir.physicalAddr);
    pageFrameMgr->freePageFrame(procInfo->kernelPageTable.physicalAddr);
    pageFrameMgr->freePageFrame(procInfo->lowerPageTable.physicalAddr);
    pageFrameMgr->freePageFrame(procInfo->upperPageTable.physicalAddr);

    // free pages
    for (int i = 0; i < procInfo->getNumPages(); ++i)
    {
        pageFrameMgr->freePageFrame(procInfo->getPage(i).physicalAddr);
    }

    // reset ProcessInfo
    procInfo->reset();
}

void ProcessMgr::logError(const char* errorMsg)
{
    screen << "Could not create process:\n" << errorMsg << '\n';
}

// create ProcessMgr instance
ProcessMgr processMgr;
