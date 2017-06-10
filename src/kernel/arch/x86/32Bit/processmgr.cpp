#include "pageframemgr.h"
#include "paging.h"
#include "processmgr.h"
#include "screen.h"
#include "string.h"
#include "system.h"

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

    /// @todo switch to process's page directory

    /// @todo allocate and map pages for code and stack

    /// @todo copy process's code

    /// @todo switch to user mode

    // allocate a process ID
    newProcInfo->id = getNewId();

    screen << "PID: " << newProcInfo->id << '\n';

    /// @todo run process

    /// @todo clean up process

    newProcInfo->id = 0;
}

bool ProcessMgr::createProcessPageDir(ProcessInfo* newProcInfo)
{
    /// @todo Instead of picking a random address, it would be
    /// better to find the end of the kernel and map the pages there.
    uintptr_t virAddr = 0xc03f'0000;
    for (int i = 0; i < ProcessInfo::NUM_PAGE_TABLES; ++i)
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
                pageFrameMgr.freePageFrame(newProcInfo->pageTables[j]);
            }

            logError("Could not allocate page frame.");
            return false;
        }

        newProcInfo->pageTables[i] = phyAddr;

        // We need to map the page dir and table to modify them.
        // Pick a temporary address above the kernel to use.
        mapPage(getKernelPageDirStart(), virAddr, phyAddr);

        virAddr += PAGE_SIZE;
    }

    uint32_t* pageDir = reinterpret_cast<uint32_t*>(newProcInfo->pageTables[0] + KERNEL_VIRTUAL_BASE);
    uint32_t* pageTableLower = reinterpret_cast<uint32_t*>(newProcInfo->pageTables[1] + KERNEL_VIRTUAL_BASE);
    uint32_t* pageTableUpper = reinterpret_cast<uint32_t*>(newProcInfo->pageTables[2] + KERNEL_VIRTUAL_BASE);
    uint32_t* pageTableKernel = reinterpret_cast<uint32_t*>(newProcInfo->pageTables[3] + KERNEL_VIRTUAL_BASE);

    // copy kernel page directory and page table
    memcpy(pageDir, getKernelPageDirStart(), PAGE_SIZE);
    /// @todo change permissions to user mode
    memcpy(pageTableKernel, getKernelPageTableStart(), PAGE_SIZE);

    // clear upper and lower memory page tables
    memset(pageTableUpper, 0, PAGE_SIZE);
    memset(pageTableLower, 0, PAGE_SIZE);

    // map lower page table in page directory
    mapPageTable(pageDir, newProcInfo->pageTables[1], 0);

    // map upper page table in page directory right before kernel page table
    int upperIdx = (KERNEL_VIRTUAL_BASE - PAGE_SIZE) >> 22;
    mapPageTable(pageDir, newProcInfo->pageTables[2], upperIdx);

    /// @todo unmap process pages from kernel page table

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
