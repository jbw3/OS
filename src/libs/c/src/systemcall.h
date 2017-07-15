#ifndef SYSTEM_CALLS_H_
#define SYSTEM_CALLS_H_

#include "stdint.h"

const uint32_t SYSTEM_CALL_WRITE       =  0;
const uint32_t SYSTEM_CALL_GETPID      =  1;
const uint32_t SYSTEM_CALL_EXIT        =  2;
const uint32_t SYSTEM_CALL_FORK        =  3;
const uint32_t SYSTEM_CALL_READ        =  4;
const uint32_t SYSTEM_CALL_SCHED_YIELD =  5;
const uint32_t SYSTEM_CALL_GETPPID     =  6;
const uint32_t SYSTEM_CALL_WAITPID     =  7;

extern "C"
uint32_t systemCallNumArgs(uint32_t sysCallNum, uint32_t numArgs, ...);

template<typename... Ts>
uint32_t systemCall(uint32_t sysCallNum, Ts... ts)
{
    return systemCallNumArgs(sysCallNum, sizeof...(ts), ts...);
}

#endif // SYSTEM_CALLS_H_
