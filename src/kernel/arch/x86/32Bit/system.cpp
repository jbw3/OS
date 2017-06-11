#include "screen.h"
#include "system.h"

extern "C"
void panic(const char* file, unsigned long line, const char* function, const char* message)
{
    screen.setBackgroundColor(os::Screen::EColor::eRed);
    screen.setForegroundColor(os::Screen::EColor::eWhite);

    screen << "Kernel panic!!!\n"
           << file << ", line " << line << '\n'
           << function << '\n'
           << message;

    /// @todo What should we do here? Maybe hang in debug
    /// mode and reboot in release mode.
    while (1);
}

int currentPrivilegeLevel()
{
    return getRegCS() & 0x3;    // bits 0 and 1 are CPL
}