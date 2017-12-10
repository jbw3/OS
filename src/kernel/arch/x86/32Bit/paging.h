#ifndef PAGING_H_
#define PAGING_H_

#include <stdint.h>

#include "isr.h"

/// @todo These constants need to be moved to pagetreex86_32.h.
#define PAGE_SIZE          4096
#define PAGE_BOUNDARY_MASK (~(PAGE_SIZE - 1))
#define PAGE_SIZE_MASK     (PAGE_SIZE - 1)

#define PAGE_DIR_NUM_ENTRIES 1024
#define PAGE_DIR_INDEX_MASK  (PAGE_DIR_NUM_ENTRIES - 1)

#define PAGE_TABLE_NUM_ENTRIES 1024
#define PAGE_TABLE_INDEX_MASK  (PAGE_TABLE_NUM_ENTRIES - 1)
#define PAGE_TABLE_INDEX_SHIFT (12)

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

#define PAGE_ERROR_PRESENT (0x1)
#define PAGE_ERROR_WRITE   (0x2)
#define PAGE_ERROR_USER    (0x4)

struct multiboot_info;

extern "C"
{

uint32_t* getKernelPageDir();

uint32_t* getKernelPageTable1();

uint32_t* getKernelPageTable2();

/**
 * @brief Sets the page directory.
 * @param pageDirAddr the physical address of the new page directory
 */
void setPageDirectory(uint32_t pageDirAddr);

/**
 * @brief Invalidate TLB for the given address.
 */
void invalidatePage(uint32_t addr);

void pageFault(const registers* regs);

} // extern "C"

void configPaging();

/**
 * @brief Map a page table in a page directory.
 */
void mapPageTable(uint32_t* pageDir, uint32_t pageTable, int pageDirIdx, bool user = false);

/**
 * @brief Map a page in a page table.
 */
void mapPage(uint32_t* pageTable, uint32_t virtualAddr, uint32_t physicalAddr, bool user = false);

/**
 * @brief Map a page in the first available page table entry and return the virtual address.
 */
bool mapPage(int pageDirIdx, uint32_t* pageTable, uint32_t& virtualAddr, uint32_t physicalAddr, bool user = false);

/**
 * @brief Unmap a page from a page table.
 */
void unmapPage(uint32_t* pageTable, uint32_t virtualAddr);

/**
 * @brief Map Multiboot modules
 */
void mapModules(const multiboot_info* mbootInfo);

#endif // PAGING_H_
