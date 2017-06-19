#include "unistd.h"
#include "systemcall.h"

extern "C"
{

ssize_t write(int fildes, const void* buf, size_t nbyte)
{
    systemCall3(SYSTEM_CALL_WRITE,
                fildes,
                reinterpret_cast<uint32_t>(buf),
                nbyte);

    /// @todo return code from system call
    return -1;
}

} // extern "C"
