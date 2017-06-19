#include "isr.h"
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

void systemCallHandler(const registers* regs)
{
    // pointer to the stack before the system call which contains the following:
    // 1. return address
    // 2. system call number
    // 3. arguments
    const uint32_t* stack = reinterpret_cast<const uint32_t*>(regs->esp);

    // system call number is on the stack
    uint32_t sysCallNum = stack[1];

    // the number of arguments is stored in eax
    uint32_t numArgs = regs->eax;

    if (sysCallNum >= SYSTEM_CALLS_SIZE || SYSTEM_CALLS[sysCallNum] == nullptr)
    {
        /// @todo Temporarily calling PANIC. This needs to
        /// be changed to something else like killing the
        /// calling process.
        PANIC("Unknown system call.");
    }
    else
    {
        const void* funcPtr = SYSTEM_CALLS[sysCallNum];

        systemCall(funcPtr, numArgs, stack + 2);
    }
}
