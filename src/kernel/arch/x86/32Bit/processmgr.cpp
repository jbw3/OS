#include "gdt.h"
#include "multiboot.h"
#include "pageframemgr.h"
#include "paging.h"
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
    numPageFrames = 0;
}

void ProcessMgr::ProcessInfo::addPageFrame(const PageFrameInfo& info)
{
    pageFrames[numPageFrames++] = info;
}

ProcessMgr::ProcessInfo::PageFrameInfo ProcessMgr::ProcessInfo::getPageFrame(int i) const
{
    return pageFrames[i];
}

int ProcessMgr::ProcessInfo::getNumPageFrames() const
{
    return numPageFrames;
}

uintptr_t* ProcessMgr::ProcessInfo::getPageDir()
{
    return reinterpret_cast<uintptr_t*>(pageFrames[0].physicalAddr + KERNEL_VIRTUAL_BASE);
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
        // copy kernel page directory
        ok = createProcessPageDir(newProcInfo);
    }

    if (ok)
    {
        // switch to process's page directory
        setPageDirectory(newProcInfo->getPageFrame(0).physicalAddr);

        // copy the program and set up the stack
        ok = setUpProgram(module, newProcInfo);
    }

    if (ok)
    {
        // allocate a process ID
        newProcInfo->id = getNewId();

        /// @todo remove this
        screen << "Created process " << newProcInfo->id << '\n';

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

void ProcessMgr::forkCurrentProcess()
{
    bool ok = true;

    // find an entry in the process info table
    ProcessInfo* newProcInfo = nullptr;
    ok = getNewProcInfo(newProcInfo);

    if (ok)
    {
        // copy kernel page directory
        ok = createProcessPageDir(newProcInfo);
    }

    if (ok)
    {
        // switch to process's page directory
        setPageDirectory(newProcInfo->getPageFrame(0).physicalAddr);

        /// @todo copy process's pages
    }

    /// @todo switch to process
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

bool ProcessMgr::createProcessPageDir(ProcessInfo* newProcInfo)
{
    /// @todo Instead of picking a random address, it would be
    /// better to find the end of the kernel and map the pages there.
    constexpr uintptr_t TEMP_VIRTUAL_ADDRESS = 0xc03f'0000;

    // Allocate a page directory and three page tables: a lower memory
    // page table (for code), an upper memory page table (right before
    // kernel for stack), and a kernel page table.
    uintptr_t virAddr = TEMP_VIRTUAL_ADDRESS;
    for (int i = 0; i < 4; ++i)
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

        newProcInfo->addPageFrame({virAddr, phyAddr});

        // We need to map the page dir and table to modify them.
        // Pick a temporary address above the kernel to use.
        mapPage(getKernelPageDirStart(), virAddr, phyAddr);

        virAddr += PAGE_SIZE;
    }

    uintptr_t* pageDir = reinterpret_cast<uintptr_t*>(TEMP_VIRTUAL_ADDRESS);
    uintptr_t* pageTableLower = reinterpret_cast<uintptr_t*>(TEMP_VIRTUAL_ADDRESS + PAGE_SIZE);
    uintptr_t* pageTableUpper = reinterpret_cast<uintptr_t*>(TEMP_VIRTUAL_ADDRESS + PAGE_SIZE * 2);
    uintptr_t* pageTableKernel = reinterpret_cast<uintptr_t*>(TEMP_VIRTUAL_ADDRESS + PAGE_SIZE * 3);

    // copy kernel page directory and page table
    memcpy(pageDir, getKernelPageDirStart(), PAGE_SIZE);
    memcpy(pageTableKernel, getKernelPageTableStart(), PAGE_SIZE);

    // clear upper and lower memory page tables
    memset(pageTableUpper, 0, PAGE_SIZE);
    memset(pageTableLower, 0, PAGE_SIZE);

    // map lower page table in page directory
    mapPageTable(pageDir, newProcInfo->getPageFrame(1).physicalAddr, 0, true);

    // map upper page table in page directory right before kernel page table
    int upperIdx = (KERNEL_VIRTUAL_BASE - PAGE_SIZE) >> 22;
    mapPageTable(pageDir, newProcInfo->getPageFrame(2).physicalAddr, upperIdx, true);

    // unmap process pages from kernel page table
    uintptr_t unmapAddr = TEMP_VIRTUAL_ADDRESS;
    for (int i = 0; i < newProcInfo->getNumPageFrames(); ++i)
    {
        unmapPage(getKernelPageDirStart(), unmapAddr);

        unmapAddr += PAGE_SIZE;
    }

    return true;
}

bool ProcessMgr::setUpProgram(const multiboot_mod_list* module, ProcessInfo* newProcInfo)
{
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

        newProcInfo->addPageFrame({virAddr, phyAddr});
        mapPage(newProcInfo->getPageDir(), virAddr, phyAddr, true);

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
    newProcInfo->addPageFrame({ProcessInfo::KERNEL_STACK_PAGE, kernelStackPhyAddr});
    mapPage(newProcInfo->getPageDir(), ProcessInfo::KERNEL_STACK_PAGE, kernelStackPhyAddr);

    // allocate and map a page for the user stack
    uintptr_t userStackPhyAddr = pageFrameMgr->allocPageFrame();
    if (userStackPhyAddr == 0)
    {
        logError("Could not allocate a page frame for the user stack.");
        return false;
    }
    newProcInfo->addPageFrame({ProcessInfo::USER_STACK_PAGE, userStackPhyAddr});
    mapPage(newProcInfo->getPageDir(), ProcessInfo::USER_STACK_PAGE, userStackPhyAddr, true);

    return true;
}

bool ProcessMgr::copyProcessPages(ProcessInfo* dstProc, const ProcessInfo* srcProc)
{
    for (int i = dstProc->getNumPageFrames(); i < srcProc->getNumPageFrames(); ++i)
    {
        // allocate a page
        uintptr_t phyAddr = pageFrameMgr->allocPageFrame();
        if (phyAddr == 0)
        {
            logError("Could not allocate a page frame for the new process.");
            return false;
        }

        /// @todo map the page
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
    // free page frames
    for (int i = 0; i < procInfo->getNumPageFrames(); ++i)
    {
        pageFrameMgr->freePageFrame(procInfo->getPageFrame(i).physicalAddr);
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
