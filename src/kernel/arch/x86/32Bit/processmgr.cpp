#include "pageframemgr.h"
#include "paging.h"
#include "processmgr.h"
#include "screen.h"
#include "string.h"
#include "system.h"

ProcessMgr::ProcessInfo::ProcessInfo()
{
    numPageFrames = 0;
}

void ProcessMgr::ProcessInfo::addPageFrame(uintptr_t addr)
{
    pageFrames[numPageFrames++] = addr;
}

uintptr_t ProcessMgr::ProcessInfo::getPageFrame(int i) const
{
    return pageFrames[i];
}

int ProcessMgr::ProcessInfo::getNumPageFrames() const
{
    return numPageFrames;
}

ProcessMgr::ProcessMgr(PageFrameMgr& pageFrameMgr) :
    pageFrameMgr(pageFrameMgr)
{
    for (int i = 0; i < MAX_NUM_PROCESSES; ++i)
    {
        processInfo[i].id = 0;
    }
}

void ProcessMgr::createProcess()
{
    bool ok = false;

    // find an entry in the process info table
    ProcessInfo* newProcInfo = nullptr;
    for (int i = 0; i < MAX_NUM_PROCESSES; ++i)
    {
        if (processInfo[i].id == 0)
        {
            newProcInfo = &processInfo[i];
            break;
        }
    }

    if (newProcInfo == nullptr)
    {
        logError("The maximum number of processes has already been created.");
        return;
    }

    // copy kernel page directory
    ok = createProcessPageDir(newProcInfo);
    if (!ok)
    {
        return;
    }

    // switch to process's page directory
    setPageDirectory(newProcInfo->getPageFrame(0));

    // copy the program and set up the stack
    setUpProgram(newProcInfo);

    /// @todo switch to user mode

    // allocate a process ID
    newProcInfo->id = getNewId();

    screen << "PID: " << newProcInfo->id << '\n';

    /// @todo run process

    /// @todo clean up process

    uintptr_t kernelPageDirPhyAddr = reinterpret_cast<uintptr_t>(getKernelPageDirStart()) - KERNEL_VIRTUAL_BASE;
    setPageDirectory(kernelPageDirPhyAddr);
    newProcInfo->id = 0;
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
        uintptr_t phyAddr = pageFrameMgr.allocPageFrame();

        // log an error if we could not get a page frame
        if (phyAddr == 0)
        {
            // free any page frames allocated up to this point
            for (int j = 0; j < i; ++j)
            {
                pageFrameMgr.freePageFrame(newProcInfo->getPageFrame(j));
            }

            logError("Could not allocate page frame.");
            return false;
        }

        newProcInfo->addPageFrame(phyAddr);

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
    /// @todo change permissions to user mode
    memcpy(pageTableKernel, getKernelPageTableStart(), PAGE_SIZE);

    // clear upper and lower memory page tables
    memset(pageTableUpper, 0, PAGE_SIZE);
    memset(pageTableLower, 0, PAGE_SIZE);

    // map lower page table in page directory
    mapPageTable(pageDir, newProcInfo->getPageFrame(1), 0);

    // map upper page table in page directory right before kernel page table
    int upperIdx = (KERNEL_VIRTUAL_BASE - PAGE_SIZE) >> 22;
    mapPageTable(pageDir, newProcInfo->getPageFrame(2), upperIdx);

    // unmap process pages from kernel page table
    uintptr_t unmapAddr = TEMP_VIRTUAL_ADDRESS;
    for (int i = 0; i < newProcInfo->getNumPageFrames(); ++i)
    {
        unmapPage(getKernelPageDirStart(), unmapAddr);

        unmapAddr += PAGE_SIZE;
    }

    return true;
}

bool ProcessMgr::setUpProgram(ProcessInfo* newProcInfo)
{
    // allocate and map pages for code and stack

    /// @todo copy process's code

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

void ProcessMgr::logError(const char* errorMsg)
{
    screen << "Could not create process:\n" << errorMsg << '\n';
}
