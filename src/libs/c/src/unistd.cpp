#include "unistd.h"
#include "systemcall.h"

extern "C"
{

ssize_t write(int fildes, const void* buf, size_t nbyte)
{
    /// @todo implement

    systemCall0(SYSTEM_CALL_WRITE);

    return -1;
}

} // extern "C"
