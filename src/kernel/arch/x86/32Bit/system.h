#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

// symbols provided in the linker script
extern uint32_t kernelVirtualBase;
extern uint32_t kernelTextOffset;
extern uint32_t kernelVirtualStart;
extern uint32_t kernelPhysicalStart;
extern uint32_t kernelVirtualEnd;
extern uint32_t kernelPhysicalEnd;

const uint32_t KERNEL_VIRTUAL_BASE   = (uint32_t)(&kernelVirtualBase);
const uint32_t KERNEL_TEXT_OFFSET    = (uint32_t)(&kernelTextOffset);
const uint32_t KERNEL_VIRTUAL_START  = (uint32_t)(&kernelVirtualStart);
const uint32_t KERNEL_PHYSICAL_START = (uint32_t)(&kernelPhysicalStart);
const uint32_t KERNEL_VIRTUAL_END    = (uint32_t)(&kernelVirtualEnd);
const uint32_t KERNEL_PHYSICAL_END   = (uint32_t)(&kernelPhysicalEnd);

uint8_t inb(uint16_t port);

void outb(uint16_t port, uint8_t value);

/**
 * @brief Clear global interrupt flag.
 */
void clearInt();

/**
 * @brief Set global interrupt flag.
 */
void setInt();

const void* getStackPointer();

const void* getStackStart();

const void* getStackEnd();

uint32_t getStackOffset();

/**
 * @brief Gets the value of the CR2 register
 */
uint32_t getRegCR2();

/**
 * @brief Switch to user mode (privilege level 3).
 */
void switchToUserMode(uintptr_t userStackAddr, uintptr_t* currentStackAddr);

/**
 * @todo Is this needed?
 *
 * @brief Initialize a new process's stack.
 * @return 1 for the new process; 0 for the original process
 */
int forkProcess(uint32_t pageDirAddr, uint32_t* currentStackAddr);

/**
 * @brief Switch to the given process's stack.
 */
void switchToProcessStack(uintptr_t newStackAddr, uintptr_t* currentStackAddr);

/**
 * @todo move this to libstdc++
 */
void __cxa_pure_virtual();

/**
 * @brief Called when something really, really bad happens...
 * @param file the file in which the error occurred
 * @param line the line on which the error occurred
 * @param function the name of the function in which the error occurred
 * @param message error message
 */
void panic(const char* file, unsigned long line, const char* function, const char* message);

#define PANIC(message) panic(__FILE__, __LINE__, __PRETTY_FUNCTION__, message)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SYSTEM_H_
