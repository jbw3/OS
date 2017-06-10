/**
 * @brief Process manager
 */

#ifndef PROCESS_MGR_H_
#define PROCESS_MGR_H_

#include <stdint.h>

/// @todo put this in a posix header
typedef unsigned int pid_t;

class PageFrameMgr;

/**
 * @brief Process manager
 */
class ProcessMgr
{
public:
    /**
     * @brief Constructor
     */
    ProcessMgr(PageFrameMgr& pageFrameMgr);

    void createProcess();

private:
    struct ProcessInfo
    {
        /// Unique ID for the process.
        pid_t id;

        constexpr static int NUM_PAGE_TABLES = 4;

        /// The physical addresses of page tables needed for the process.
        /// The four addesses point to the page directory, lower memory
        /// page table (for code), upper memory page table (right before
        /// kernel for stack), and the kernel page table, respectively.
        uintptr_t pageTables[NUM_PAGE_TABLES];
    };

    constexpr static int MAX_NUM_PROCESSES = 4;
    ProcessInfo processInfo[MAX_NUM_PROCESSES];

    PageFrameMgr& pageFrameMgr;

    /**
     * @brief Create a new page directory for a process by copying
     * the kernel page directory.
     */
    bool createProcessPageDir(ProcessInfo* newProcInfo);

    /**
     * @brief Get an ID for a new process.
     */
    pid_t getNewId();

    /**
     * @brief Log an error message.
     */
    void logError(const char* errorMsg);
};

#endif // PROCESS_MGR_H_
