#include "screen.h"

extern "C"
void panic(const char* file, unsigned long line, const char* function, const char* message)
{
    screen.setBackgroundColor(VgaDriver::EColor::eRed);
    screen.setForegroundColor(VgaDriver::EColor::eWhite);

    screen << "Kernel panic!!!\n"
           << file << ", line " << line << '\n'
           << function << '\n'
           << message;

    /// @todo What should we do here? Maybe hang in debug
    /// mode and reboot in release mode.
    while (1);
}
