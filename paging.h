#ifndef PAGING_H_
#define PAGING_H_

#include <stdint.h>

#include "irq.h"

extern "C"
{

const uint32_t* getPageDirStart();

const uint32_t* getPageDirEnd();

/**
 * @brief Initialize the page directory
 */
void initPageDir();

/**
 * @brief Initialize a page table
 * @brief addr the address of the page table (MUST be 4 KiB aligned)
 */
void initPageTable(void* addr);

void enablePaging();

void pageFault(const registers* regs);

} // extern "C"

void initPaging();

#endif // PAGING_H_
