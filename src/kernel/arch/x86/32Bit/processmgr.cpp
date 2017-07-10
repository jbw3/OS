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
    parentId = 0;
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
    currentProcIdx(0),
    pageFrameMgr(nullptr),
    mbootInfo(nullptr)
{
}

void ProcessMgr::setPageFrameMgr(PageFrameMgr* pageFrameMgrPtr)
{
    pageFrameMgr = pageFrameMgrPtr;
}

void ProcessMgr::setMultibootInfo(const multiboot_info* multibootInfo)
{
    mbootInfo = multibootInfo;
}

void ProcessMgr::mainloop()
{
    ProcessInfo* proc = nullptr;

    const multiboot_mod_list* initModule = nullptr;
    bool found = findModule("init", initModule);
    if (!found)
    {
        PANIC("Could not find init program.");
    }

    // kick off init process
    createProcess(initModule);
    proc = &processInfo[currentProcIdx];

    while (true)
    {
        // switch to kernel's page directory
        uintptr_t kernelPageDirPhyAddr = reinterpret_cast<uintptr_t>(getKernelPageDirStart()) - KERNEL_VIRTUAL_BASE;
        setPageDirectory(kernelPageDirPhyAddr);

        // enable interrupts
        setInt();

        // process actions
        switch (procAction)
        {
        case EAction::eNone:
            // do nothing
            break;

        case EAction::eFork:
        {
            ProcessInfo* newProc = forkProcess(actionProc);
            if (newProc == nullptr)
            {
                actionProc->actionResult.pid = -1;
            }
            else
            {
                actionProc->actionResult.pid = newProc->id;
                newProc->actionResult.pid = 0;
            }
            break;
        }

        case EAction::eYield:
            proc = getNextScheduledProcess();
            break;

        case EAction::eExit:
            cleanUpProcess(actionProc);
            proc = getNextScheduledProcess();
            break;
        }

        // reset action
        procAction = EAction::eNone;

        if (proc != nullptr)
        {
            // switch to process
            switchToProcessFromKernel(proc);
        }
        else
        {
            // halt if there are no more processes
            asm volatile ("hlt");
        }
    }
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

pid_t ProcessMgr::forkCurrentProcess()
{
    procAction = EAction::eFork;
    actionProc = getCurrentProcessInfo();

    // switch to kernel
    switchToKernelFromProcess();

    // we resume here after the fork is complete
    return getCurrentProcessInfo()->actionResult.pid;
}

void ProcessMgr::yieldCurrentProcess()
{
    procAction = EAction::eYield;
    actionProc = getCurrentProcessInfo();

    // switch to kernel
    switchToKernelFromProcess();
}

void ProcessMgr::exitCurrentProcess()
{
    procAction = EAction::eExit;
    actionProc = getCurrentProcessInfo();

    // switch to kernel
    switchToKernelFromProcess();
}

ProcessMgr::ProcessInfo* ProcessMgr::getCurrentProcessInfo()
{
    return *ProcessInfo::PROCESS_INFO;
}

bool ProcessMgr::findModule(const char* name, const multiboot_mod_list*& module)
{
    bool found = false;
    uint32_t addr = mbootInfo->mods_addr + KERNEL_VIRTUAL_BASE;
    uint32_t count = mbootInfo->mods_count;

    for (uint32_t i = 0; i < count; ++i)
    {
        // get module info struct
        const multiboot_mod_list* modulePtr = reinterpret_cast<const multiboot_mod_list*>(addr);

        // check if name matches
        const char* modName = reinterpret_cast<const char*>(modulePtr->cmdline + KERNEL_VIRTUAL_BASE);
        if (strcmp(name, modName) == 0)
        {
            module = modulePtr;
            found = true;
            break;
        }

        addr += sizeof(multiboot_mod_list);
    }

    return found;
}

ProcessMgr::ProcessInfo* ProcessMgr::forkProcess(ProcessInfo* procInfo)
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

        // copy process page tables
        copyProcessPageTables(newProcInfo, procInfo);

        // copy the stack pointer
        newProcInfo->stack = procInfo->stack;

        // copy process's pages
        ok = copyProcessPages(newProcInfo, procInfo);
    }

    // unmap process pages from kernel page table
    unmapPages(newProcInfo, getKernelPageTableStart());

    if (ok)
    {
        // allocate a process ID
        newProcInfo->id = getNewId();

        // set parent process ID
        newProcInfo->parentId = procInfo->id;

        // switch to process's page directory
        setPageDirectory(newProcInfo->pageDir.physicalAddr);

        // set the ProcessInfo pointer
        *ProcessInfo::PROCESS_INFO = newProcInfo;
    }
    else
    {
        // if we get here, something went wrong and we need to
        // clean things up

        cleanUpProcess(newProcInfo);
    }

    // switch back to kernel's page directory
    uintptr_t kernelPageDirPhyAddr = reinterpret_cast<uintptr_t>(getKernelPageDirStart()) - KERNEL_VIRTUAL_BASE;
    setPageDirectory(kernelPageDirPhyAddr);

    return ok ? newProcInfo : nullptr;
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
    for (int i = 0; i < srcProc->getNumPages(); ++i)
    {
        ProcessInfo::PageFrameInfo srcPageInfo = srcProc->getPage(i);

        // allocate a page
        uintptr_t dstPhyAddr = pageFrameMgr->allocPageFrame();
        if (dstPhyAddr == 0)
        {
            logError("Could not allocate a page frame for the new process.");
            return false;
        }
        uintptr_t virAddr = srcPageInfo.virtualAddr;
        dstProc->addPage({virAddr, dstPhyAddr});

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
        mapPage(dstPageTable, virAddr, dstPhyAddr, true);

        // temporarily map the pages in the kernel's page table so we can copy
        uintptr_t dstTempAddr = 0;
        uintptr_t srcTempAddr = 0;
        bool ok = mapPage((KERNEL_VIRTUAL_BASE >> 22), getKernelPageTableStart(), dstTempAddr, dstPhyAddr);
        if (!ok)
        {
            logError("Could not map destination temporary page.");
            return false;
        }
        ok = mapPage((KERNEL_VIRTUAL_BASE >> 22), getKernelPageTableStart(), srcTempAddr, srcPageInfo.physicalAddr);
        if (!ok)
        {
            logError("Could not map source temporary page.");
            return false;
        }

        // copy the page
        memcpy(reinterpret_cast<void*>(dstTempAddr), reinterpret_cast<const void*>(srcTempAddr), PAGE_SIZE);

        // unmap the temporary pages from the kernel's page table
        unmapPage(getKernelPageTableStart(), dstTempAddr);
        unmapPage(getKernelPageTableStart(), srcTempAddr);
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

        // make sure the ID is a positive number
        if (nextId <= 0)
        {
            nextId = 1;
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

ProcessMgr::ProcessInfo* ProcessMgr::getNextScheduledProcess()
{
    currentProcIdx = (currentProcIdx == MAX_NUM_PROCESSES - 1) ? 0 : currentProcIdx + 1;

    bool wrapped = false;
    int startIdx = currentProcIdx;
    while (currentProcIdx != startIdx || !wrapped)
    {
        if (processInfo[currentProcIdx].id != 0)
        {
            return &processInfo[currentProcIdx];
        }

        if (currentProcIdx == MAX_NUM_PROCESSES - 1)
        {
            currentProcIdx = 0;
            wrapped = true;
        }
        else
        {
            ++currentProcIdx;
        }
    }

    return nullptr;
}

void ProcessMgr::switchToKernelFromProcess()
{
    // switch to kernel
    switchToProcessStack(kernelStack, &getCurrentProcessInfo()->stack);
}

void ProcessMgr::switchToProcessFromKernel(ProcessInfo* procInfo)
{
    // switch to process's page directory
    setPageDirectory(procInfo->pageDir.physicalAddr);

    // set the kernel stack for the process
    setKernelStack(ProcessInfo::KERNEL_STACK_START);

    // switch to process
    switchToProcessStack(procInfo->stack, &kernelStack);
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
