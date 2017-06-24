#include "screen.h"
#include "system.h"
#include "systemcalls.h"
#include "unistd.h"

namespace systemcall
{

ssize_t write(int fildes, const void* buf, size_t nbyte)
{
    // only support stdout right now
    if (fildes != STDOUT_FILENO)
    {
        return -1;
    }

    const char* charPtr = reinterpret_cast<const char*>(buf);
    for (size_t i = 0; i < nbyte; ++i)
    {
        screen.write(charPtr[i]);
    }

    return 0;
}

void test()
{
    screen << "system call test\n";
}

void test3(int arg1, int arg2, int arg3)
{
    screen << "system call test: " << arg1 << ", " << arg2 << ", " << arg3 << '\n';
}

} // namespace systemcall

constexpr uint32_t SYSTEM_CALLS_SIZE = 11;
const void* SYSTEM_CALLS[SYSTEM_CALLS_SIZE] = {
    reinterpret_cast<const void*>(systemcall::write),
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    reinterpret_cast<const void*>(systemcall::test),
    reinterpret_cast<const void*>(systemcall::test3),
};

extern "C"
uint32_t systemCallHandler(uint32_t sysCallNum, uint32_t numArgs, const uint32_t* argPtr)
{
    if (sysCallNum >= SYSTEM_CALLS_SIZE || SYSTEM_CALLS[sysCallNum] == nullptr)
    {
        /// @todo Temporarily calling PANIC. This needs to
        /// be changed to something else like killing the
        /// calling process.
        PANIC("Unknown system call.");

        return 0xbad'ca11;
    }
    else
    {
        const void* funcPtr = SYSTEM_CALLS[sysCallNum];

        return execSystemCall(funcPtr, numArgs, argPtr);
    }
}
