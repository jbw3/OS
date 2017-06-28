/**
 * @brief Process manager
 */

#ifndef PROCESS_MGR_H_
#define PROCESS_MGR_H_

#include <stdint.h>
#include <unistd.h>

struct multiboot_mod_list;
class PageFrameMgr;

/**
 * @brief Process manager
 */
class ProcessMgr
{
private:
    /**
     * @brief Stores information about a process.
     */
    class ProcessInfo
    {
    public:
        constexpr static uintptr_t CODE_VIRTUAL_START = 0;
        constexpr static int MAX_NUM_PAGE_FRAMES = 10;

        /// virtual address of the kernel stack page
        static const uintptr_t KERNEL_STACK_PAGE;

        /// virtual address of the user stack page
        static const uintptr_t USER_STACK_PAGE;

        /// the ProcessInfo instance for the current process
        static ProcessInfo** PROCESS_INFO;

        /// the start address of the kernel stack page
        static const uintptr_t KERNEL_STACK_START;

        /// Unique ID for the process.
        pid_t id;

        ProcessInfo();

        void reset();

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

public:
    /**
     * @brief Constructor
     */
    ProcessMgr();

    void setPageFrameMgr(PageFrameMgr* pageFrameMgrPtr);

    void createProcess(const multiboot_mod_list* module);

    void forkCurrentProcess();

    void exitCurrentProcess();

    /**
     * @brief Get the ProcessInfo for calling process.
     */
    ProcessInfo* getCurrentProcessInfo();

private:
    constexpr static int MAX_NUM_PROCESSES = 4;
    ProcessInfo processInfo[MAX_NUM_PROCESSES];

    /// the page frame manager
    PageFrameMgr* pageFrameMgr;

    /// the kernel stack before switching to a process
    uintptr_t kernelStack;

    /**
     * @brief Gets an empty entry in the process info table.
     * @return true if an entry was available; false, otherwise
     */
    bool getNewProcInfo(ProcessInfo*& procInfo);

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
     * @brief Copy code and stack pages from one process to another.
     */
    bool copyProcessPages(ProcessInfo* dstProc, const ProcessInfo* srcProc);

    /**
     * @brief Get an ID for a new process.
     */
    pid_t getNewId();

    /**
     * @brief Find the ProcessInfo for the given process ID. If the process ID
     * cannot be found, a null pointer is returned.
     */
    ProcessInfo* findProcess(pid_t id);

    /**
     * @brief Clean up resources used by a process.
     */
    void cleanUpProcess(ProcessInfo* procInfo);

    /**
     * @brief Log an error message.
     */
    void logError(const char* errorMsg);
};

extern ProcessMgr processMgr;

#endif // PROCESS_MGR_H_
