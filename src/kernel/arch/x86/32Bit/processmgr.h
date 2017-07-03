/**
 * @brief Process manager
 */

#ifndef PROCESS_MGR_H_
#define PROCESS_MGR_H_

#include <stdint.h>
#include <unistd.h>

#include "paging.h"

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
        constexpr static int MAX_NUM_PAGES = 8;

        /// virtual address of the kernel stack page
        static const uintptr_t KERNEL_STACK_PAGE;

        /// virtual address of the user stack page
        static const uintptr_t USER_STACK_PAGE;

        /// the ProcessInfo instance for the current process
        static ProcessInfo** PROCESS_INFO;

        /// the start address of the kernel stack page
        static const uintptr_t KERNEL_STACK_START;

        struct PageFrameInfo
        {
            uintptr_t virtualAddr;
            uintptr_t physicalAddr;
        };

        /// Unique ID for the process.
        pid_t id;

        ProcessInfo();

        void reset();

        void addPage(const PageFrameInfo& info);

        PageFrameInfo getPage(int i) const;

        int getNumPages() const;

        PageFrameInfo pageDir;

        PageFrameInfo kernelPageTable;

        PageFrameInfo lowerPageTable;

        PageFrameInfo upperPageTable;

    private:
        /// The addresses of pages used by the process for code, data,
        /// and stack.
        PageFrameInfo pages[MAX_NUM_PAGES];

        int numPages;
    };

public:
    /**
     * @brief Constructor
     */
    ProcessMgr();

    void setPageFrameMgr(PageFrameMgr* pageFrameMgrPtr);

    void createProcess(const multiboot_mod_list* module);

    bool forkCurrentProcess();

    void exitCurrentProcess();

    /**
     * @brief Get the ProcessInfo for calling process.
     */
    ProcessInfo* getCurrentProcessInfo();

private:
    /// @todo auto-map pages instead of using these hard-coded address
    constexpr static uintptr_t PAGE_DIR_VIRTUAL_ADDRESS = 0xc03f'0000;
    constexpr static uintptr_t PAGE_TABLE1_VIRTUAL_ADDRESS = PAGE_DIR_VIRTUAL_ADDRESS + PAGE_SIZE * 1;
    constexpr static uintptr_t PAGE_TABLE2_VIRTUAL_ADDRESS = PAGE_DIR_VIRTUAL_ADDRESS + PAGE_SIZE * 2;
    constexpr static uintptr_t PAGE_TABLE3_VIRTUAL_ADDRESS = PAGE_DIR_VIRTUAL_ADDRESS + PAGE_SIZE * 3;
    constexpr static uintptr_t TEMP_VIRTUAL_ADDRESS = PAGE_DIR_VIRTUAL_ADDRESS + PAGE_SIZE * 4;

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
     * @brief Copying the kernel page directory and table to the given process.
     */
    void copyKernelPageDir(ProcessInfo* newProcInfo);

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
