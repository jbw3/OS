#include "sys/wait.h"
#include "systemcall.h"

extern "C"
{

pid_t wait(int* stat_loc)
{
    return waitpid(-1, stat_loc, 0);
}

pid_t waitpid(pid_t pid, int* stat_loc, int options)
{
    return systemCall(SYSTEM_CALL_WAITPID, pid, stat_loc, options);
}

} // extern "C"
