#include "fcntl.h"

#include "systemcall.h"

extern "C"
{

int open(const char* path, int oflag, ...)
{
    return systemCall(SYSTEM_CALL_OPEN, path, oflag);
}

} // extern "C"
