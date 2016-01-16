/**
 * @brief Debugging functions
 */

#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdint.h>

#include "multiboot.h"

/**
 * @brief Debugging function to print Multiboot info
 * @param mbootInfo pointer to Multiboot info struct
 */
void printMultibootInfo(const multiboot_info* mbootInfo);

void printMemMap(uint32_t addr, uint32_t len);

void printMem(int ptr);

void printMem(const uint32_t* ptr);

#endif // _DEBUG_H
