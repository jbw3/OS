#include "processmgr.h"
#include "screen.h"
#include "system.h"

ProcessMgr::ProcessMgr(PageFrameMgr* pageFrameMgr) :
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
        screen << "Could not create process:\nThe maximum number of processes has already been created.\n";
        return;
    }

    /// @todo copy kernel page directory

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

void ProcessMgr::createProcessPageDir(ProcessInfo* newProcInfo)
{

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
