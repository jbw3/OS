#include "stdarg.h"
#include "unistd.h"

#include "systemcall.h"

extern "C"
{

int dup(int fildes)
{
    return systemCall(SYSTEM_CALL_DUP, fildes);
}

int execl(const char* path, const char* arg0, ...)
{
    constexpr size_t MAX_ARGS = 32;
    /// @todo dynamically allocate argv so we can support more args
    char* argv[MAX_ARGS];

    va_list args;
    va_start(args, arg0);

    // We have to get rid of the const to conform to the posix argument standard.
    // However, the argument is not modified, so this is OK.
    argv[0] = const_cast<char*>(arg0);

    // copy args
    size_t idx = 1;
    char* arg = va_arg(args, char*);
    while (arg != nullptr && idx < MAX_ARGS - 1)
    {
        argv[idx++] = arg;
        arg = va_arg(args, char*);
    }

    argv[idx] = nullptr;

    va_end(args);

    return execv(path, argv);
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

pid_t getpid()
{
    return systemCall(SYSTEM_CALL_GETPID);
}

pid_t getppid()
{
    return systemCall(SYSTEM_CALL_GETPPID);
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
