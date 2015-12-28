#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include "irq.h"

namespace os
{

class Keyboard
{
public:
    static void init();

    static void interruptHandler(const registers* regs);

private:
    /// US keyboard layout
    static const uint16_t LAYOUT_US[128];

    // control keys that are currently pressed (shift, alt, etc.)
    static uint16_t controlPressed;

    static void keyRelease(uint16_t key);

    static void keyPress(uint16_t key);

    static char shift(char ch);
};

} // namespace os

#endif // KEYBOARD_H_
