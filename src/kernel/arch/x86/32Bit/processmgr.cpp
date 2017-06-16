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

ProcessMgr::ProcessInfo::ProcessInfo()
{
    reset();
}

void ProcessMgr::ProcessInfo::reset()
{
    id = 0;
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

uintptr_t* ProcessMgr::ProcessInfo::getPageDir()
{
    return reinterpret_cast<uintptr_t*>(pageFrames[0] + KERNEL_VIRTUAL_BASE);
}

ProcessMgr::ProcessMgr(PageFrameMgr& pageFrameMgr) :
    pageFrameMgr(pageFrameMgr)
{
}

void ProcessMgr::createProcess(const multiboot_mod_list* module)
{
    bool ok = true;

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
        ok = false;
    }

    if (ok)
    {
        // copy kernel page directory
        ok = createProcessPageDir(newProcInfo);
    }

    if (ok)
    {
        // switch to process's page directory
        setPageDirectory(newProcInfo->getPageFrame(0));

        // copy the program and set up the stack
        ok = setUpProgram(module, newProcInfo);
    }

    if (ok)
    {
        // allocate a process ID
        newProcInfo->id = getNewId();

        /// @todo remove this
        screen << "PID: " << newProcInfo->id << '\n';

        // set the kernel stack for the process
        setKernelStack(ProcessInfo::KERNEL_STACK_PAGE + PAGE_SIZE - 4);

        // switch to user mode and run process (this does not return)
        switchToUserMode(ProcessInfo::USER_STACK_PAGE + PAGE_SIZE - 4);
    }

    // if we get here, something went wrong (ok == false) and we need to
    // clean things up

    // switch back to kernel's page directory
    uintptr_t kernelPageDirPhyAddr = reinterpret_cast<uintptr_t>(getKernelPageDirStart()) - KERNEL_VIRTUAL_BASE;
    setPageDirectory(kernelPageDirPhyAddr);

    // free page frames
    for (int i = 0; i < newProcInfo->getNumPageFrames(); ++i)
    {
        pageFrameMgr.freePageFrame(newProcInfo->getPageFrame(i));
    }

    // reset ProcessInfo
    newProcInfo->reset();
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
    memcpy(pageTableKernel, getKernelPageTableStart(), PAGE_SIZE);

    // clear upper and lower memory page tables
    memset(pageTableUpper, 0, PAGE_SIZE);
    memset(pageTableLower, 0, PAGE_SIZE);

    // map lower page table in page directory
    mapPageTable(pageDir, newProcInfo->getPageFrame(1), 0, true);

    // map upper page table in page directory right before kernel page table
    int upperIdx = (KERNEL_VIRTUAL_BASE - PAGE_SIZE) >> 22;
    mapPageTable(pageDir, newProcInfo->getPageFrame(2), upperIdx, true);

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
        uintptr_t phyAddr = pageFrameMgr.allocPageFrame();
        if (phyAddr == 0)
        {
            logError("Could not allocate page frame.");
            return false;
        }

        newProcInfo->addPageFrame(phyAddr);
        mapPage(newProcInfo->getPageDir(), virAddr, phyAddr, true);

        virAddr += PAGE_SIZE;
    }

    // copy process's code
    size_t codeSize = module->mod_end - module->mod_start;
    memcpy(reinterpret_cast<void*>(ProcessInfo::CODE_VIRTUAL_START),
           reinterpret_cast<const void*>(module->mod_start + KERNEL_VIRTUAL_BASE),
           codeSize);

    // allocate and map a page for the kernel stack
    uintptr_t kernelStackPhyAddr = pageFrameMgr.allocPageFrame();
    if (kernelStackPhyAddr == 0)
    {
        logError("Could not allocate a page frame for the kernel stack.");
        return false;
    }
    newProcInfo->addPageFrame(kernelStackPhyAddr);
    mapPage(newProcInfo->getPageDir(), ProcessInfo::KERNEL_STACK_PAGE, kernelStackPhyAddr);

    // allocate and map a page for the user stack
    uintptr_t userStackPhyAddr = pageFrameMgr.allocPageFrame();
    if (userStackPhyAddr == 0)
    {
        logError("Could not allocate a page frame for the user stack.");
        return false;
    }
    newProcInfo->addPageFrame(userStackPhyAddr);
    mapPage(newProcInfo->getPageDir(), ProcessInfo::USER_STACK_PAGE, userStackPhyAddr, true);

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
