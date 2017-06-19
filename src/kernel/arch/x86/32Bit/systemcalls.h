#ifndef SYSTEM_CALLS_H_
#define SYSTEM_CALLS_H_

#include "stdint.h"

struct registers;

/**
 * @brief System call interrupt handler.
 */
void systemCallHandler(const registers* regs);

extern "C"
uint32_t systemCall(const void* funcPtr, uint32_t numArgs, const uint32_t* argsPtr);

#endif // SYSTEM_CALLS_H_
