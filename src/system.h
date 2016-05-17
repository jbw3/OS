#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <stdint.h>

#define KERNEL_VIRTUAL_BASE 0xC0000000

#ifdef __cplusplus
extern "C"
{
#endif

uint8_t inb(uint16_t port);

void outb(uint16_t port, uint8_t value);

const void* getStackPointer();

const void* getStackStart();

const void* getStackEnd();

uint32_t getStackOffset();

uint32_t getKernelPhysicalEnd();

/**
 * @brief Gets the value of the CR2 register
 */
uint32_t getRegCR2();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SYSTEM_H_
