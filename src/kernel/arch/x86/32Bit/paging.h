#ifndef PAGING_H_
#define PAGING_H_

#include <stdint.h>

#include "isr.h"

#define PAGE_SIZE          4096
#define PAGE_BOUNDARY_MASK (~(PAGE_SIZE - 1))
#define PAGE_SIZE_MASK     (PAGE_SIZE - 1)

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
 * @brief Add a page table to the page directory
 * @todo remove
 */
void addPageTable(int idx, uint32_t pageTableAddr);

/**
 * @brief Add a page to a page table
 * @todo remove
 */
void addPage(uint32_t pageAddr);

/**
 * @brief Map a page in the page table
 */
void mapPage(const uint32_t* pageDir, uint32_t virtualAddr, uint32_t physicalAddr);

#endif // PAGING_H_