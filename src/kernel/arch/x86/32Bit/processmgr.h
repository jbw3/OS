/**
 * @brief Process manager
 */

#ifndef PROCESS_MGR_H_
#define PROCESS_MGR_H_

#include <stdint.h>

/// @todo put this in a posix header
typedef unsigned int pid_t;

struct multiboot_mod_list;
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

    void createProcess(const multiboot_mod_list* module);

private:
    class ProcessInfo
    {
    public:
        constexpr static uintptr_t CODE_VIRTUAL_START = 0;
        constexpr static int MAX_NUM_PAGE_FRAMES = 8;
        static const uintptr_t STACK_VIRTUAL_START;

        /// Unique ID for the process.
        pid_t id;

        ProcessInfo();

        void addPageFrame(uintptr_t addr);

        uintptr_t getPageFrame(int i) const;

        int getNumPageFrames() const;

        uintptr_t* getPageDir();

    private:
        /// The physical addresses of page frames used by the process.
        /// The first page in this array is the process's page
        /// directory.
        uintptr_t pageFrames[MAX_NUM_PAGE_FRAMES];

        int numPageFrames;
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
     * @brief Set up the program for the process by copying the
     * code and setting up the stack.
     */
    bool setUpProgram(const multiboot_mod_list* module, ProcessInfo* newProcInfo);

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