#ifndef SYSTEM_CALLS_H_
#define SYSTEM_CALLS_H_

#include "stdint.h"

/**
 * @brief System call interrupt handler.
 */
extern "C"
uint32_t systemCallHandler(uint32_t sysCallNum, uint32_t numArgs, const uint32_t* argPtr);

extern "C"
uint32_t systemCall(const void* funcPtr, uint32_t numArgs, const uint32_t* argsPtr);

#endif // SYSTEM_CALLS_H_
