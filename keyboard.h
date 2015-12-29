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

    static void processQueue();

private:
    /// US keyboard layout
    static const uint16_t LAYOUT_US[128];

    // control keys that are currently pressed (shift, alt, etc.)
    static uint16_t controlPressed;

    // queue of keys to process
    static const unsigned int QUEUE_SIZE = 32;
    static uint8_t queue[QUEUE_SIZE];
    static unsigned int qHead;
    static unsigned int qTail;

    static void keyRelease(uint16_t key);

    static void keyPress(uint16_t key);

    static char shift(char ch);

    static void updateLights();
};

} // namespace os

#endif // KEYBOARD_H_
