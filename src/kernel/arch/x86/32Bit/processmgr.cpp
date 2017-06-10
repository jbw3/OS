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
    createProcessPageDir(newProcInfo);

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
    // get page frames for the process's page dir and page table
    // (these are physical addresses)
    uintptr_t pageDirPhy = pageFrameMgr.allocPageFrame();
    uintptr_t pageTablePhy = pageFrameMgr.allocPageFrame();

    if (pageDirPhy == 0 || pageTablePhy == 0)
    {
        logError("Could not allocate page frame.");
        return false;
    }

    // We need to map the page dir and table to modify them.
    // Pick a temporary address above the kernel to use.
    /// @todo Instead of picking a random address, it would be
    /// better to find the end of the kernel and map the pages there.
    uintptr_t pageDirVir = 0xc03f'0000;
    uintptr_t pageTableVir = pageDirVir + PAGE_SIZE;

    mapPage(getKernelPageDirStart(), pageDirVir, pageDirPhy);
    mapPage(getKernelPageDirStart(), pageTableVir, pageTablePhy);

    // clear page dir and table
    memset(reinterpret_cast<void*>(pageDirVir), 0, PAGE_SIZE);
    memset(reinterpret_cast<void*>(pageTableVir), 0, PAGE_SIZE);

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
