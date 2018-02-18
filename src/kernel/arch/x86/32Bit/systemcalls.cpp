#include "keyboard.h"
#include "processmgr.h"
#include "screen.h"
#include "streamtable.h"
#include "sys/wait.h"
#include "system.h"
#include "systemcalls.h"
#include "unistd.h"

namespace systemcall
{

void clearTerminal()
{
    screen.clear();
}

void configTerminal(int background, int foreground)
{
    if (background >= 0 && background <= 15)
    {
        screen.setBackgroundColor(static_cast<VgaDriver::EColor>(background));
    }

    if (foreground >= 0 && foreground <= 15)
    {
        screen.setForegroundColor(static_cast<VgaDriver::EColor>(foreground));
    }
}

int execv(const char* path, char* const argv[])
{
    processMgr.switchCurrentProcessExecutable(path, argv);

    // we should only get here if an error occurred
    return -1;
}

void exit(int status)
{
    processMgr.exitCurrentProcess(status);
}

pid_t fork()
{
    return processMgr.forkCurrentProcess();
}

uint32_t getKey()
{
    uint16_t key = 0;
    bool found = false;
    while (!found)
    {
        found = os::Keyboard::getKey(key);
        if (!found)
        {
            processMgr.yieldCurrentProcess();
        }
    }

    return key;
}

int getNumModules()
{
    return processMgr.getNumModules();
}

void getModuleName(int index, char* name)
{
    processMgr.getModuleName(index, name);
}

pid_t getpid()
{
    return processMgr.getCurrentProcessInfo()->getId();
}

pid_t getppid()
{
    return processMgr.getCurrentProcessInfo()->parentProcess->getId();
}

ssize_t read(int fildes, void* buf, size_t nbyte)
{
    // convert the local stream index to the master stream table index
    int masterStreamIdx = processMgr.getCurrentProcessInfo()->getStreamIndex(fildes);
    if (masterStreamIdx < 0)
    {
        return -1;
    }

    // look up the stream in the master stream table
    Stream* stream = streamTable.getStream(masterStreamIdx);
    if (stream == nullptr)
    {
        return -1;
    }

    // read from the stream
    ssize_t rv = stream->read(reinterpret_cast<uint8_t*>(buf), nbyte);

    return rv;
}

int sched_yield()
{
    processMgr.yieldCurrentProcess();

    return 0;
}

pid_t waitpid(pid_t pid, int* stat_loc, int options)
{
    if ( options & (WCONTINUED | WUNTRACED) )
    {
        /// @todo support these options
        return -1;
    }

    if (pid == 0 || pid < -1)
    {
        /// @todo support these options
        return -1;
    }

    bool hang = !(options & WNOHANG);

    ProcessMgr::ProcessInfo* proc = processMgr.getCurrentProcessInfo();
    ProcessMgr::ProcessInfo* child = nullptr;
    do
    {
        for (size_t i = 0; i < proc->childProcesses.getSize(); ++i)
        {
            ProcessMgr::ProcessInfo* p = proc->childProcesses[i];

            // check if the child has terminated
            if (p->getStatus() == ProcessMgr::ProcessInfo::eTerminated)
            {
                if (pid == -1 || pid == p->getId())
                {
                    child = p;
                    break;
                }
            }
        }

        if (child == nullptr && hang)
        {
            // if no child process was found, yield so we don't waste the CPU
            // (a CPU is a terrible thing to waste)
            processMgr.yieldCurrentProcess();
        }
    } while (child == nullptr && hang);

    if (child == nullptr)
    {
        return 0;
    }
    else
    {
        if (stat_loc != nullptr)
        {
            /// @todo encode more info in stat_loc (as required by posix)
            *stat_loc = child->exitCode;
        }

        // save ID before we clean up the process
        pid_t childId = child->getId();

        // clean up process
        processMgr.cleanUpCurrentProcessChild(child);

        return childId;
    }
}

ssize_t write(int fildes, const void* buf, size_t nbyte)
{
    // convert the local stream index to the master stream table index
    int masterStreamIdx = processMgr.getCurrentProcessInfo()->getStreamIndex(fildes);
    if (masterStreamIdx < 0)
    {
        return -1;
    }

    // look up the stream in the master stream table
    Stream* stream = streamTable.getStream(masterStreamIdx);
    if (stream == nullptr)
    {
        return -1;
    }

    // write to the stream
    ssize_t rv = stream->write(reinterpret_cast<const uint8_t*>(buf), nbyte);

    return rv;
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
    reinterpret_cast<const void*>(systemcall::waitpid),
    reinterpret_cast<const void*>(systemcall::execv),
    reinterpret_cast<const void*>(systemcall::getNumModules),
    reinterpret_cast<const void*>(systemcall::getModuleName),
    reinterpret_cast<const void*>(systemcall::configTerminal),
    reinterpret_cast<const void*>(systemcall::clearTerminal),
    reinterpret_cast<const void*>(systemcall::getKey),
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
