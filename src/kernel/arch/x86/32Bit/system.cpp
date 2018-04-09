#include "kernellogger.h"
#include "userlogger.h"

extern "C"
void panic(const char* file, unsigned long line, const char* function, const char* message)
{
    klog.logError("Panic", "Kernel panic!!!");
    klog.logError("Panic", "{}, line {}", file, line);
    klog.logError("Panic", function);
    klog.logError("Panic", message);

    ulog.log("\x1b[31;47mKernel panic!!!\n"
             "{}, line {}\n"
             "{}\n"
             "{}\x1b[0m\n",
             file, line, function, message);

    /// @todo What should we do here? Maybe hang in debug
    /// mode and reboot in release mode.
    while (true);
}
