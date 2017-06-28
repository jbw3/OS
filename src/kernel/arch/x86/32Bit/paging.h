#ifndef PAGING_H_
#define PAGING_H_

#include <stdint.h>

#include "isr.h"

#define PAGE_SIZE          4096
#define PAGE_BOUNDARY_MASK (~(PAGE_SIZE - 1))
#define PAGE_SIZE_MASK     (PAGE_SIZE - 1)

#define PAGE_DIR_NUM_ENTRIES 1024
#define PAGE_DIR_INDEX_MASK  (PAGE_DIR_NUM_ENTRIES - 1)

#define PAGE_TABLE_NUM_ENTRIES 1024
#define PAGE_TABLE_INDEX_MASK  (PAGE_DIR_NUM_ENTRIES - 1)

#define PAGE_DIR_ADDRESS       0xFFFFF000
#define PAGE_DIR_PAGE_SIZE     0x00000080
#define PAGE_DIR_ACCESSED      0x00000020
#define PAGE_DIR_CACHE_DISABLE 0x00000010
#define PAGE_DIR_WRITE_THROUGH 0x00000008
#define PAGE_DIR_USER          0x00000004
#define PAGE_DIR_READ_WRITE    0x00000002
#define PAGE_DIR_PRESENT       0x00000001

#define PAGE_TABLE_ADDRESS       0xFFFFF000
#define PAGE_TABLE_GLOBAL        0x00000100
#define PAGE_TABLE_DIRTY         0x00000040
#define PAGE_TABLE_ACCESSED      0x00000020
#define PAGE_TABLE_CACHE_DISABLE 0x00000010
#define PAGE_TABLE_WRITE_THROUGH 0x00000008
#define PAGE_TABLE_USER          0x00000004
#define PAGE_TABLE_READ_WRITE    0x00000002
#define PAGE_TABLE_PRESENT       0x00000001

extern "C"
{

uint32_t* getKernelPageDirStart();

uint32_t* getKernelPageDirEnd();

/**
 * @brief Initialize the page directory
 */
void initPageDir();

/**
 * @brief Initialize a page table
 * @brief addr the address of the page table (MUST be 4 KiB aligned)
 */
void initPageTable(uint32_t addr);

void enablePaging();

void disablePaging();

bool isPagingEnabled();

void pageFault(const registers* regs);

} // extern "C"

void configPaging();

/**
 * @brief Map a page in the page table
 */
void mapPage(const uint32_t* pageDir, uint32_t virtualAddr, uint32_t physicalAddr);

/**
 * @brief Returns a pointer that can be used to access the page table
 * identified by the given index in the given page directory.
 * since the PDE stores a physical address of the page table,
 * this function transforms this address into a virtual one that
 * can be used directly.
 */
uint32_t* getPageTablePointer(uint32_t* pageDir, uint16_t index);

/**
 * Utilities for page table walking and information retrieval
 */
namespace mem {

/**
 * @brief Returns the number of available kernel page directory entries
 */
uint32_t kNumAvailablePDEs();

/**
 * @brief Returns the number of available page directory entries
 * for the specified page directory.
 * @param pageDir points to the page directory to use
 * @param startIdx is the index into the page directory at which
 * to begin searching
 */
uint32_t numAvailablPDEs(uint32_t* pageDir, uint16_t startIdx=0);

/**
 * @brief Returns last kernel PDE in the list of those currently in use,
 * assuming the PDEs are allocated in order and that the first entry exists.
 */
uint16_t lastUsedKernelPDEIndex();

}

#endif // PAGING_H_
