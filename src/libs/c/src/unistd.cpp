#include "unistd.h"
#include "systemcall.h"

extern "C"
{

ssize_t write(int fildes, const void* buf, size_t nbyte)
{
    ssize_t rc = systemCallNumArgs(SYSTEM_CALL_WRITE, 3,
                            fildes,
                            reinterpret_cast<uint32_t>(buf),
                            nbyte);
    return rc;
}

} // extern "C"
