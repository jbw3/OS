#include "unistd.h"
#include "systemcall.h"

extern "C"
{

pid_t getpid()
{
    return systemCall(SYSTEM_CALL_GETPID);
}

pid_t getppid()
{
    return systemCall(SYSTEM_CALL_GETPPID);
}

int execv(const char* path, char* const argv[])
{
    return systemCall(SYSTEM_CALL_EXECV,
                      path,
                      argv);
}

pid_t fork()
{
    return systemCall(SYSTEM_CALL_FORK);
}

ssize_t read(int fildes, void* buf, size_t nbyte)
{
    ssize_t rc = systemCall(SYSTEM_CALL_READ,
                            fildes,
                            buf,
                            nbyte);
    return rc;
}

ssize_t write(int fildes, const void* buf, size_t nbyte)
{
    ssize_t rc = systemCall(SYSTEM_CALL_WRITE,
                            fildes,
                            buf,
                            nbyte);
    return rc;
}

} // extern "C"
