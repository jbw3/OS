/**
 * @brief Process manager
 */

#ifndef PROCESS_MGR_H_
#define PROCESS_MGR_H_

#include <stdint.h>
#include <unistd.h>

#include "paging.h"
#include "set.hpp"

struct multiboot_mod_list;
class PageFrameMgr;

/**
 * @brief Process manager
 */
class ProcessMgr
{
public:
    /**
     * @brief Stores information about a process.
     */
    class ProcessInfo
    {
    public:
        constexpr static uintptr_t CODE_VIRTUAL_START = 0;
        constexpr static int MAX_NUM_PAGES = 8;
        constexpr static size_t MAX_NUM_CHILDREN = 32;
        constexpr static int MAX_NUM_STREAM_INDICES = 8;

        /// virtual address of the kernel stack page
        static const uintptr_t KERNEL_STACK_PAGE;

        /// virtual address of the user stack page
        static const uintptr_t USER_STACK_PAGE;

        /// the ProcessInfo instance for the current process
        static ProcessInfo** PROCESS_INFO;

        /// the start address of the kernel stack page
        static const uintptr_t KERNEL_STACK_START;

        static ProcessInfo* initProcess;

        enum EStatus
        {
            /// The process is currently running.
            eRunning,

            /// The process has been terminated.
            eTerminated,
        };

        struct PageFrameInfo
        {
            uintptr_t virtualAddr;
            uintptr_t physicalAddr;

            enum eType
            {
                eOther,
                eCode,
                eData,
                eStack,
            } type;
        };

        /// Process's parent process.
        ProcessInfo* parentProcess;

        /// Process's child processes.
        Set<ProcessInfo*, MAX_NUM_CHILDREN> childProcesses;

        /// Exit code
        int exitCode;

        ProcessInfo();

        void reset();

        void start(pid_t pid);

        void exit();

        void addPage(const PageFrameInfo& info);

        PageFrameInfo getPage(int i) const;

        int getNumPages() const;

        int getNumPagesOfType(PageFrameInfo::eType type) const;

        pid_t getId() const;

        EStatus getStatus() const;

        int addStreamIndex(int masterStreamIdx);

        void removeStreamIndex(int procStreamIdx);

        int getStreamIndex(int procStreamIdx) const;

        void copyStreamIndices(ProcessInfo* procInfo);

        int duplicateStreamIndex(int procStreamIdx);

        int duplicateStreamIndex(int procStreamIdx, int dupProcStreamIdx);

        PageFrameInfo pageDir;

        PageFrameInfo kernelPageTable;

        PageFrameInfo lowerPageTable;

        PageFrameInfo upperPageTable;

        /// saves the process's stack before switching to another process
        uintptr_t stack;

        /// an action's result
        union ActionResult
        {
            pid_t pid;
        } actionResult;

    private:
        /// Unique ID for the process.
        pid_t id;

        /// The addresses of pages used by the process for code, data,
        /// and stack.
        PageFrameInfo pages[MAX_NUM_PAGES];

        /// Maps the process's stream indices (e.g. file descriptors)
        /// to the kernel's stream table.
        int streamIndices[MAX_NUM_STREAM_INDICES];

        int numPages;

        EStatus status;
    };

    /// The tag used in the kernel log
    static const char* LOG_TAG;

    /**
     * @brief Constructor
     */
    ProcessMgr();

    void setPageFrameMgr(PageFrameMgr* pageFrameMgrPtr);

    void setMultibootInfo(const multiboot_info* multibootInfo);

    void mainloop();

    /// @todo make this private
    void createProcess(const multiboot_mod_list* module, int stdinStreamIdx, int stdoutStreamIdx, int stderrStreamIdx);

    pid_t forkCurrentProcess();

    /**
     * @brief Switch the executable a process is running.
     */
    bool switchCurrentProcessExecutable(const char* path, const char* const argv[]);

    void yieldCurrentProcess();

    void exitCurrentProcess(int exitCode);

    void cleanUpCurrentProcessChild(ProcessInfo* childProc);

    void processTimerInterrupt(const registers* regs);

    /**
     * @brief Get the ProcessInfo for calling process.
     */
    ProcessInfo* getCurrentProcessInfo();

    uint32_t getNumModules() const;

    bool getModuleName(uint32_t index, char* name) const;

private:
    constexpr static int MAX_NUM_PROCESSES = 32;
    ProcessInfo processes[MAX_NUM_PROCESSES];

    Set<ProcessInfo*, MAX_NUM_PROCESSES> runningProcs;

    size_t currentProcIdx;

    /// whether interrupt process switching is enabled
    bool intSwitchEnabled;

    enum class EAction
    {
        /// perform no action
        eNone,

        /// fork a process
        eFork,

        /// yield a process
        eYield,

        /// exit a process
        eExit,
    };

    /// the page frame manager
    PageFrameMgr* pageFrameMgr;

    /// the multiboot info
    const multiboot_info* mbootInfo;

    /// the kernel stack before switching to a process
    uintptr_t kernelStack;

    /// an action to perform on a process
    EAction procAction;

    /// the process to perform the action on
    ProcessInfo* actionProc;

    /**
     * @brief Find the multiboot module with the given name.
     */
    bool findModule(const char* name, const multiboot_mod_list*& module);

    /**
     * @brief Fork the given process.
     */
    ProcessInfo* forkProcess(ProcessInfo* procInfo);

    /**
     * @brief Set up args on stack.
     */
    uintptr_t copyArgs(const char* const argv[], uintptr_t stackEnd);

    /**
     * @brief Gets an empty entry in the process info table.
     * @return true if an entry was available; false, otherwise
     */
    bool getNewProcInfo(ProcessInfo*& procInfo);

    /**
     * @brief Allocates all pages needed for a new process's paging
     * structure. Also, temporarily maps them in the given page
     * directory, so they can be accessed.
     */
    bool initPaging(ProcessInfo* procInfo, uintptr_t* pageTable);

    /**
     * @brief Unmap temporarily mapped pages.
     */
    void unmapPages(ProcessInfo* procInfo, uintptr_t* pageTable);

    /**
     * @brief Copy the kernel page directory and table to the given process.
     */
    void copyKernelPageTable(ProcessInfo* dstProc, uintptr_t* srcKernelPageTable);

    /**
     * @brief Create a process's page tables to map its code and stack.
     */
    void createProcessPageTables(ProcessInfo* newProcInfo);

    /**
     * @brief Copy a process's page tables for code and stack from another
     * process.
     */
    void copyProcessPageTables(ProcessInfo* dstProc, ProcessInfo* srcProc);

    /**
     * @brief Set up the program for the process by copying the
     * code and setting up the stack.
     */
    bool setUpProgram(const multiboot_mod_list* module, ProcessInfo* newProcInfo);

    /**
     * @brief Copy code and stack pages from one process to another.
     */
    bool copyProcessPages(ProcessInfo* dstProc, ProcessInfo* srcProc);

    /**
     * @brief Get an ID for a new process.
     */
    pid_t getNewId();

    /**
     * @brief Get the next scheduled process.
     */
    ProcessInfo* getNextScheduledProcess();

    /**
     * @brief Execute an action.
     */
    void executeAction(EAction action, ProcessInfo* process);

    /**
     * @brief Switch to the kernel from the current process.
     */
    void switchToKernelFromProcess();

    /**
     * @brief Switch to the given process from the kernel.
     */
    void switchToProcessFromKernel(ProcessInfo* procInfo);

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
