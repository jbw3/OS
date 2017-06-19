#ifndef SYSTEM_CALLS_H_
#define SYSTEM_CALLS_H_

#include "stdint.h"

const uint32_t SYSTEM_CALL_WRITE = 0;

#ifdef __cplusplus
extern "C"
{
#endif

int systemCall0(uint32_t num);

int systemCall3(uint32_t num, uint32_t param1, uint32_t param2, uint32_t param3);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SYSTEM_CALLS_H_
