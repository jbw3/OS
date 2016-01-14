#ifndef PAGING_H_
#define PAGING_H_

#include <stdint.h>

#include "irq.h"

#define PAGE_DIR_ADDRESS 0xFFFFF000
#define PAGE_DIR_PRESENT 0x00000001

#define PAGE_TABLE_ADDRESS 0xFFFFF000
#define PAGE_TABLE_PRESENT 0x00000001

extern "C"
{

uint32_t* getPageDirStart();

uint32_t* getPageDirEnd();

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

void pageFault(const registers* regs);

} // extern "C"

void initPaging();

/**
 * @brief Add a page table to the page directory
 */
void addPageTable(int idx, uint32_t pageTableAddr);

/**
 * @brief Add a page to a page table
 */
void addPage(uint32_t pageAddr);

#endif // PAGING_H_
