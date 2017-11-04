#include "sched.h"
#include "systemcall.h"

extern "C"
{

int sched_yield()
{
    return systemCall(SYSTEM_CALL_SCHED_YIELD);
}

} // extern "C"
