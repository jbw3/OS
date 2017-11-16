#pragma once

#include <stdint.h>
#include "pageframemgr.h"

namespace mem {

/**
 * @class PageTable encapsulates a page table and provides
 * functionality to query and modify a page table.
 */
class PageTable
{
public:

    /**
     * @brief Default constructor. Currently used only for cases
     * when uninitialized variables have to be declared and then
     * initialized later on (since we don't have malloc()
     * capability yet).
     */
    PageTable();

    /**
     * @brief Constructs a new PageTable instance that refers
     * to the page table at the given virtual address.
     *
     * @param pageDirIdx is the index in pageDir of the PDE that
     * points to this page table
     * @param pageDir points to the page directory that refers
     * to this page table. Defaults to the kernel page directory
     */
    PageTable(uint16_t pageDirIdx, uint32_t* pageDir=nullptr);

    /**
     * @brief Initializes the kernel PageTablePointer struct. This must
     * be done before any PageTable instances are used.
     */
    static void initKernelPageTablePointer();

    /**
     * @brief initPTPageTable creates the "page table" PageTable instance.
     * If you do not know what this is DO NOT USE THIS FUNCTION! The page table
     * PageTable is reserved ahead of time for creating new page tables, since
     * there is a bit of a problem trying to create a new page table when you
     * have already run out of space in the current one.
     */
    static void initPTPageTable(uint32_t* ptPageTable, uint16_t pageDirIdx);

    /**
     * @brief Gets the page table PageTable singleton
     */
    static PageTable* getPTPageTable();

    /**
     * @brief Returns a (virtual) pointer to this page table
     */
    uint32_t* getPointer();

    /**
     * @brief Returns a (virtual) pointer to this page table's
     * page directory
     */
    uint32_t* getPageDirPointer();

    /**
     * @brief Returns the index of this page table into its parent
     * page directory
     */
    uint16_t getPageDirIndex();

    /**
     * @brief Returns true if this page table has no active PTEs
     */
    bool isEmpty();

    /**
     * @brief Returns true if this page table has no free slots
     * for new PTEs
     */
    bool isFull();

    /**
     * @brief Clears the entire page table by marking each page
     * table entry as available
     */
    void clearPageTable();

    /**
     * @brief Initializes this page table as a new page table
     * by clearing it and mapping its page frame to an accessible
     * virtual address in the PTPT. This function should not be
     * called until the PDE that points to this page table's page
     * frame has been set up.
     */
    void init();

    /**
     * @brief Returns the index of the next available page in the
     * page table.
     */
    uint16_t nextAvailablePage();

    /**
     * @brief Maps the next available virtual page to the page
     * frame containing the given physical address.
     * @param physAddr the physical address to map
     * @returns the virtual address now mapped to the given
     * physical address
     */
    uint32_t mapNextAvailablePageToAddress(uint32_t physAddr);

    /**
     * @brief Maps the page at the given physical address to
     * the virtual address corresponding to the given page
     * index. The page index is the index into this page table
     * for the desired PTE.
     * @returns a virtual pointer to the given physical address,
     * or nullptr if unable to map the page.
     */
    uint32_t* mapPage(uint16_t pageIdx, uint32_t physAddr);

    /**
     * @brief Returns the virtual address that corresponds to
     * the base of the page at the given page index.
     */
    uint32_t virtAddressOfPage(uint16_t pageIdx);

    /**
     * @brief Returns the base address of the physical page
     * frame to which the page at the given index is mapped.
     */
    uint32_t physAddressOfPageFrame(uint16_t pageIdx);

    /**
     * @brief Returns true if the given physical address is
     * mapped to one of the pages in this page table.
     * @param physAddr is the physical address
     * @param virtAddr is set to the virtual address mapped
     * to the given physical address, if such a mapping exists.
     * If this function returns false, virtAddr is not valid.
     */
    bool isMapped(uint32_t physAddr, uint32_t& virtAddr);

private:
    /**
     * @brief PageTable constructor specifically for the "page table" PageTable instance.
     * This constructor was made private to ensure that it doesn't get accidentally called
     * in the future.
     * @param ptPageTable - the virtual address of the PTPageTable (whose physical page is
     * mapped by some existing page table...)
     */
    PageTable(uint32_t* ptPageTable, uint16_t pageDirIdx);

    /**
     * @brief Returns the index into the page table pointer array
     * for this page table.
     */
    int ptPtrArrayIndex();

    /**
     * @brief Sets the page table pointer for this table from the
     * page table pointer array, based on the pageDirIdx
     */
    void setPageTablePointer();

    uint32_t* _pageDir;
    uint16_t _pageDirIdx;
    uint32_t* _pageTable;
};

}