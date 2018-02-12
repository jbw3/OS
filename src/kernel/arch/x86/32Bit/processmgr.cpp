#include "gdt.h"
#include "irq.h"
#include "multiboot.h"
#include "pageframemgr.h"
#include "processmgr.h"
#include "screen.h"
#include "string.h"
#include "system.h"
#include "utils.h"

const uintptr_t ProcessMgr::ProcessInfo::KERNEL_STACK_PAGE = KERNEL_VIRTUAL_BASE - PAGE_SIZE;
const uintptr_t ProcessMgr::ProcessInfo::USER_STACK_PAGE = ProcessMgr::ProcessInfo::KERNEL_STACK_PAGE - PAGE_SIZE;

// put the ProcessInfo pointer at the base of the kernel stack
ProcessMgr::ProcessInfo** ProcessMgr::ProcessInfo::PROCESS_INFO = reinterpret_cast<ProcessMgr::ProcessInfo**>(ProcessMgr::ProcessInfo::KERNEL_STACK_PAGE + PAGE_SIZE - 4);

// the kernel stack starts right after the ProcessInfo pointer
const uintptr_t ProcessMgr::ProcessInfo::KERNEL_STACK_START = reinterpret_cast<uintptr_t>(ProcessMgr::ProcessInfo::PROCESS_INFO) - sizeof(ProcessMgr::ProcessInfo::PROCESS_INFO);

ProcessMgr::ProcessInfo* ProcessMgr::ProcessInfo::initProcess = nullptr;

ProcessMgr::ProcessInfo::ProcessInfo()
{
    reset();
}

void ProcessMgr::ProcessInfo::reset()
{
    id = 0;
    parentProcess = nullptr;
    childProcesses.clear();
    exitCode = -1;
    pageDir = {0, 0, PageFrameInfo::eOther};
    kernelPageTable = {0, 0, PageFrameInfo::eOther};
    lowerPageTable = {0, 0, PageFrameInfo::eOther};
    upperPageTable = {0, 0, PageFrameInfo::eOther};
    numPages = 0;
    status = eTerminated;

    for (int i = 0; i < MAX_NUM_STREAM_INDICES; ++i)
    {
        streamIndices[i] = -1;
    }
}

void ProcessMgr::ProcessInfo::start(pid_t pid)
{
    id = pid;
    status = eRunning;
}

void ProcessMgr::ProcessInfo::exit()
{
    // change the parent of all children to the init process
    for (size_t i = 0; i < childProcesses.getSize(); ++i)
    {
        ProcessInfo* child = childProcesses[i];

        child->parentProcess = initProcess;
        initProcess->childProcesses.add(child);
    }

    childProcesses.clear();

    status = eTerminated;
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

int ProcessMgr::ProcessInfo::getNumPagesOfType(PageFrameInfo::eType type) const
{
    int count = 0;
    for (int i = 0; i < numPages; ++i)
    {
        if (pages[i].type == type)
        {
            ++count;
        }
    }
    return count;
}

pid_t ProcessMgr::ProcessInfo::getId() const
{
    return id;
}

ProcessMgr::ProcessInfo::EStatus ProcessMgr::ProcessInfo::getStatus() const
{
    return status;
}

int ProcessMgr::ProcessInfo::addStreamIndex(int masterStreamIdx)
{
    for (int i = 0; i < MAX_NUM_STREAM_INDICES; ++i)
    {
        if (streamIndices[i] == -1)
        {
            streamIndices[i] = masterStreamIdx;
            return i;
        }
    }

    return -1;
}

void ProcessMgr::ProcessInfo::removeStreamIndex(int procStreamIdx)
{
    if (procStreamIdx >= 0 && procStreamIdx < MAX_NUM_STREAM_INDICES)
    {
        streamIndices[procStreamIdx] = -1;
    }
}

int ProcessMgr::ProcessInfo::getStreamIndex(int procStreamIdx) const
{
    if (procStreamIdx >= 0 && procStreamIdx < MAX_NUM_STREAM_INDICES)
    {
        return streamIndices[procStreamIdx];
    }

    return -1;
}

void ProcessMgr::ProcessInfo::copyStreamIndices(ProcessInfo* procInfo)
{
    memcpy(streamIndices, procInfo->streamIndices, MAX_NUM_STREAM_INDICES * sizeof(int));
}

int ProcessMgr::ProcessInfo::duplicateStreamIndex(int procStreamIdx)
{
    if (procStreamIdx < 0 || procStreamIdx >= MAX_NUM_STREAM_INDICES)
    {
        return -1;
    }

    int masterStreamIdx = streamIndices[procStreamIdx];
    if (masterStreamIdx < 0)
    {
        return -1;
    }

    int dupStreamIdx = addStreamIndex(masterStreamIdx);
    return dupStreamIdx;
}

int ProcessMgr::ProcessInfo::duplicateStreamIndex(int procStreamIdx, int dupProcStreamIdx)
{
    if (procStreamIdx < 0 || procStreamIdx >= MAX_NUM_STREAM_INDICES || dupProcStreamIdx < 0 || dupProcStreamIdx >= MAX_NUM_STREAM_INDICES)
    {
        return -1;
    }

    int masterStreamIdx = streamIndices[procStreamIdx];
    if (procStreamIdx < 0)
    {
        return -1;
    }

    int oldMasterStreamIdx = streamIndices[dupProcStreamIdx];
    if (oldMasterStreamIdx >= 0 && procStreamIdx != dupProcStreamIdx)
    {
        /// @todo close the stream when closing is implemented
    }

    streamIndices[dupProcStreamIdx] = masterStreamIdx;
    return dupProcStreamIdx;
}

ProcessMgr::ProcessMgr() :
    currentProcIdx(0),
    intSwitchEnabled(false),
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
    createProcess(initModule, 0, 1, 1);
    proc = ProcessInfo::initProcess = runningProcs[currentProcIdx];

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
                actionProc->actionResult.pid = newProc->getId();
                newProc->actionResult.pid = 0;
            }
            break;
        }

        case EAction::eYield:
            proc = getNextScheduledProcess();
            break;

        case EAction::eExit:
            runningProcs.remove(actionProc);
            actionProc->exit();
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

void ProcessMgr::createProcess(const multiboot_mod_list* module, int stdinStreamIdx, int stdoutStreamIdx, int stderrStreamIdx)
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
        // add stream for stdin, stdout, and stderr
        newProcInfo->addStreamIndex(stdinStreamIdx);
        newProcInfo->addStreamIndex(stdoutStreamIdx);
        newProcInfo->addStreamIndex(stderrStreamIdx);

        /// @todo temp hardcode
        newProcInfo->addStreamIndex(2);

        // allocate a process ID and start the process
        newProcInfo->start(getNewId());

        // set the ProcessInfo pointer
        *ProcessInfo::PROCESS_INFO = newProcInfo;

        // add process to list of running processes
        runningProcs.add(newProcInfo);

        // set the kernel stack for the process
        setKernelStack(ProcessInfo::KERNEL_STACK_START);

        // allow process switching once interrupts are enabled again
        // in the process
        clearInt();
        intSwitchEnabled = true;

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
    executeAction(EAction::eFork, getCurrentProcessInfo());

    // we resume here after the fork is complete
    return getCurrentProcessInfo()->actionResult.pid;
}

bool ProcessMgr::switchCurrentProcessExecutable(const char* path, const char* const argv[])
{
    bool ok = true;

    ProcessInfo* procInfo = getCurrentProcessInfo();
    uintptr_t* lowerPageTable = reinterpret_cast<uintptr_t*>(procInfo->lowerPageTable.virtualAddr);

    // find module
    const multiboot_mod_list* module = nullptr;
    ok = findModule(path, module);

    if (ok)
    {
        // get size of new executable
        size_t exeSize = module->mod_end - module->mod_start;

        // get current allocated memory size for code
        int numCodePages = procInfo->getNumPagesOfType(ProcessInfo::PageFrameInfo::eCode);
        size_t allocMem = numCodePages * PAGE_SIZE;

        // find last code page
        uintptr_t lastCodePageVirAddr = 0;
        for (int i = 0; i < procInfo->getNumPages(); ++i)
        {
            const ProcessInfo::PageFrameInfo& info = procInfo->getPage(i);
            if (info.type == ProcessInfo::PageFrameInfo::eCode && info.virtualAddr > lastCodePageVirAddr)
            {
                lastCodePageVirAddr = info.virtualAddr;
            }
        }
        uintptr_t virAddr = lastCodePageVirAddr + PAGE_SIZE;

        // if we don't have enough memory for code, allocate more
        while (allocMem < exeSize)
        {
            uintptr_t phyAddr = pageFrameMgr->allocPageFrame();
            if (phyAddr == 0)
            {
                logError("Could not allocate page frame");
                return false;
            }

            procInfo->addPage({virAddr, phyAddr, ProcessInfo::PageFrameInfo::eCode});
            mapPage(lowerPageTable, virAddr, phyAddr, true);

            virAddr += PAGE_SIZE;
            allocMem += PAGE_SIZE;
        }

        /// @todo if we have more memory than we need, dealloc pages

        // copy args
        uintptr_t stackStart = copyArgs(argv, ProcessInfo::USER_STACK_PAGE + PAGE_SIZE - 4);

        // copy new executable
        memcpy(reinterpret_cast<void*>(ProcessInfo::CODE_VIRTUAL_START),
               reinterpret_cast<const void*>(module->mod_start + KERNEL_VIRTUAL_BASE),
               exeSize);

        // switch to user mode
        uintptr_t temp;
        switchToUserMode(stackStart, &temp);
    }

    return ok;
}

void ProcessMgr::yieldCurrentProcess()
{
    executeAction(EAction::eYield, getCurrentProcessInfo());
}

void ProcessMgr::exitCurrentProcess(int exitCode)
{
    ProcessInfo* currentProc = getCurrentProcessInfo();
    currentProc->exitCode = exitCode;

    executeAction(EAction::eExit, currentProc);
}

void ProcessMgr::cleanUpCurrentProcessChild(ProcessInfo* childProc)
{
    ProcessInfo* currentProc = getCurrentProcessInfo();

    currentProc->childProcesses.remove(childProc);
    cleanUpProcess(childProc);
}

void ProcessMgr::processTimerInterrupt(const registers* regs)
{
    if (intSwitchEnabled)
    {
        sendPicEoi(regs);

        yieldCurrentProcess();
    }
}

ProcessMgr::ProcessInfo* ProcessMgr::getCurrentProcessInfo()
{
    return *ProcessInfo::PROCESS_INFO;
}

uint32_t ProcessMgr::getNumModules() const
{
    return mbootInfo->mods_count;
}

bool ProcessMgr::getModuleName(uint32_t index, char* name) const
{
    if (index >= mbootInfo->mods_count)
    {
        name[0] = '\0';
        return false;
    }

    const multiboot_mod_list* moduleList = reinterpret_cast<const multiboot_mod_list*>(mbootInfo->mods_addr + KERNEL_VIRTUAL_BASE);

    const char* modName = reinterpret_cast<const char*>(moduleList[index].cmdline + KERNEL_VIRTUAL_BASE);
    strcpy(name, modName);

    return true;
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

        // copy process's streams
        newProcInfo->copyStreamIndices(procInfo);
    }

    // unmap process pages from kernel page table
    if (newProcInfo != nullptr)
    {
        unmapPages(newProcInfo, getKernelPageTableStart());
    }

    if (ok)
    {
        // allocate a process ID and start the process
        newProcInfo->start(getNewId());

        // set parent process
        newProcInfo->parentProcess = procInfo;

        // add new process to parent's children list
        procInfo->childProcesses.add(newProcInfo);

        // switch to process's page directory
        setPageDirectory(newProcInfo->pageDir.physicalAddr);

        // set the ProcessInfo pointer
        *ProcessInfo::PROCESS_INFO = newProcInfo;

        // add process to list of running processes
        runningProcs.add(newProcInfo);
    }
    else
    {
        // if we get here, something went wrong and we need to
        // clean things up

        if (newProcInfo != nullptr)
        {
            cleanUpProcess(newProcInfo);
        }
    }

    // switch back to kernel's page directory
    uintptr_t kernelPageDirPhyAddr = reinterpret_cast<uintptr_t>(getKernelPageDirStart()) - KERNEL_VIRTUAL_BASE;
    setPageDirectory(kernelPageDirPhyAddr);

    return ok ? newProcInfo : nullptr;
}

uintptr_t ProcessMgr::copyArgs(const char* const argv[], uintptr_t stackEnd)
{
    /// @todo dynamically allocate these temp arrays
    char tempArgStrings[256];
    char* tempArgPtrs[64];

    char* tempArgPtr = tempArgStrings;
    int numArgs = 0;
    size_t strSize = 0;
    for (size_t i = 0; argv[i] != nullptr; ++i)
    {
        size_t argLen = strlen(argv[i]) + 1; // add 1 for null char

        // the strings we are copying might be on the stack we are copying to,
        // so we make temporary copies here
        strcpy(tempArgPtr, argv[i]);
        tempArgPtrs[numArgs] = tempArgPtr;
        tempArgPtr += argLen;

        strSize += argLen;
        ++numArgs;
    }

    // calculate addresses of arg strings and pointers to the strings
    uintptr_t strStart = align(stackEnd - strSize, sizeof(void*), false);
    uintptr_t ptrStart = strStart - sizeof(void*) * (numArgs + 1); // add 1 for null pointer at end

    char* strPtr = reinterpret_cast<char*>(strStart);
    char** ptrPtr = reinterpret_cast<char**>(ptrStart);

    // copy strings and pointers
    for (int i = 0; i < numArgs; ++i)
    {
        *ptrPtr = strPtr;

        strcpy(strPtr, tempArgPtrs[i]);

        ++ptrPtr;
        strPtr += strlen(tempArgPtrs[i]) + 1; // add 1 for null char
    }

    // add null pointer at end
    *ptrPtr = nullptr;

    // add argc and argv on the stack
    uintptr_t* argvPtr = reinterpret_cast<uintptr_t*>(ptrStart - sizeof(void*));
    int* argcPtr = reinterpret_cast<int*>(ptrStart - 2 * sizeof(void*));

    *argvPtr = ptrStart;
    *argcPtr = numArgs;

    return ptrStart - 2 * sizeof(void*);
}

bool ProcessMgr::getNewProcInfo(ProcessInfo*& procInfo)
{
    procInfo = nullptr;
    for (int i = 0; i < MAX_NUM_PROCESSES; ++i)
    {
        if (processes[i].getId() == 0)
        {
            procInfo = &processes[i];
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
            procInfo->pageDir = {virAddr, phyAddr, ProcessInfo::PageFrameInfo::eOther};
            break;
        case 1:
            procInfo->kernelPageTable = {virAddr, phyAddr, ProcessInfo::PageFrameInfo::eOther};
            break;
        case 2:
            procInfo->lowerPageTable = {virAddr, phyAddr, ProcessInfo::PageFrameInfo::eOther};
            break;
        case 3:
            procInfo->upperPageTable = {virAddr, phyAddr, ProcessInfo::PageFrameInfo::eOther};
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

        newProcInfo->addPage({virAddr, phyAddr, ProcessInfo::PageFrameInfo::eCode});
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
    newProcInfo->addPage({ProcessInfo::KERNEL_STACK_PAGE, kernelStackPhyAddr, ProcessInfo::PageFrameInfo::eStack});
    mapPage(upperPageTable, ProcessInfo::KERNEL_STACK_PAGE, kernelStackPhyAddr);

    // allocate and map a page for the user stack
    uintptr_t userStackPhyAddr = pageFrameMgr->allocPageFrame();
    if (userStackPhyAddr == 0)
    {
        logError("Could not allocate a page frame for the user stack.");
        return false;
    }
    newProcInfo->addPage({ProcessInfo::USER_STACK_PAGE, userStackPhyAddr, ProcessInfo::PageFrameInfo::eStack});
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
        dstProc->addPage({virAddr, dstPhyAddr, srcPageInfo.type});

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
            if (processes[i].getId() == nextId)
            {
                ++nextId;
                invalidId = true;
                break;
            }
        }
    } while (invalidId);

    return nextId++;
}

ProcessMgr::ProcessInfo* ProcessMgr::getNextScheduledProcess()
{
    if (runningProcs.getSize() > 0)
    {
        currentProcIdx = (currentProcIdx >= runningProcs.getSize() - 1) ? 0 : currentProcIdx + 1;
        return runningProcs[currentProcIdx];
    }
    else
    {
        return nullptr;
    }
}

void ProcessMgr::executeAction(EAction action, ProcessInfo* process)
{
    procAction = action;
    actionProc = process;

    // switch to kernel
    switchToKernelFromProcess();
}

void ProcessMgr::switchToKernelFromProcess()
{
    // don't try to switch processes while we're in the kernel
    intSwitchEnabled = false;

    // switch to kernel
    switchToProcessStack(kernelStack, &getCurrentProcessInfo()->stack);

    // we're back from the kernel, enable process switching again
    intSwitchEnabled = true;
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
