#ifndef SYSTEM_CALLS_H_
#define SYSTEM_CALLS_H_

#include "stdint.h"

const uint32_t SYSTEM_CALL_WRITE =  0;
const uint32_t SYSTEM_CALL_TEST  =  9;
const uint32_t SYSTEM_CALL_TEST3 = 10;

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t systemCallNumArgs(uint32_t sysCallNum, uint32_t numArgs, ...);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SYSTEM_CALLS_H_
