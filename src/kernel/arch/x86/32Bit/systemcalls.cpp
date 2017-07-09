#include "keyboard.h"
#include "processmgr.h"
#include "screen.h"
#include "system.h"
#include "systemcalls.h"
#include "unistd.h"

namespace systemcall
{

void exit(int /*status*/)
{
    processMgr.exitCurrentProcess();
}

pid_t fork()
{
    return processMgr.forkCurrentProcess();
}

pid_t getpid()
{
    return processMgr.getCurrentProcessInfo()->id;
}

pid_t getppid()
{
    return processMgr.getCurrentProcessInfo()->parentId;
}

ssize_t read(int fildes, void* buf, size_t nbyte)
{
    // only support stdin right now
    if (fildes != STDIN_FILENO)
    {
        return -1;
    }

    os::Keyboard::processQueue();

    char ch;
    size_t idx = 0;
    while (idx < nbyte && screen.read(ch))
    {
        reinterpret_cast<char*>(buf)[idx] = ch;
        ++idx;
    }

    return static_cast<ssize_t>(idx);
}

int sched_yield()
{
    processMgr.yieldCurrentProcess();

    return 0;
}

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

} // namespace systemcall

constexpr uint32_t SYSTEM_CALLS_SIZE = 16;
const void* SYSTEM_CALLS[SYSTEM_CALLS_SIZE] = {
    reinterpret_cast<const void*>(systemcall::write),
    reinterpret_cast<const void*>(systemcall::getpid),
    reinterpret_cast<const void*>(systemcall::exit),
    reinterpret_cast<const void*>(systemcall::fork),
    reinterpret_cast<const void*>(systemcall::read),
    reinterpret_cast<const void*>(systemcall::sched_yield),
    reinterpret_cast<const void*>(systemcall::getppid),
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
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
