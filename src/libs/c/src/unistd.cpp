#include "unistd.h"
#include "systemcall.h"

extern "C"
{

ssize_t write(int fildes, const void* buf, size_t nbyte)
{
    /// @todo implement

    // systemCall3(SYSTEM_CALL_WRITE,
    //             fildes,
    //             reinterpret_cast<uint32_t>(buf),
    //             nbyte);
    systemCall3(10, 10, 52, 12345);

    return -1;
}

} // extern "C"
