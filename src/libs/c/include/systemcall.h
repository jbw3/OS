#ifndef SYSTEM_CALLS_H_
#define SYSTEM_CALLS_H_

#include "stdint.h"

const uint32_t SYSTEM_CALL_WRITE =  0;
const uint32_t SYSTEM_CALL_TEST  =  9;
const uint32_t SYSTEM_CALL_TEST3 = 10;

extern "C"
uint32_t systemCallNumArgs(uint32_t sysCallNum, uint32_t numArgs, ...);

template<typename... Ts>
uint32_t systemCall(uint32_t sysCallNum, Ts... ts)
{
    return systemCallNumArgs(sysCallNum, sizeof...(ts), ts...);
}

#endif // SYSTEM_CALLS_H_