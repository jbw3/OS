#ifndef KEYBOARD_H_
#define KEYBOARD_H_

#include "irq.h"

namespace os
{

/// @todo some of the code in this class needs an interrupt guard
class Keyboard
{
public:
    static void init();

    /**
     * @brief The keyboard interrupt handler.
     */
    static void interruptHandler(const registers* regs);

    /**
     * @brief Get the next key-press from the queue.
     */
    static bool getKey(uint16_t& key);

    /**
     * @brief Get the next ASCII character key from the queue.
     */
    static bool getChar(char& ch);

    static void processQueue();

private:
    /// US keyboard layout
    static const uint16_t LAYOUT_US[128];

    // control keys that are currently pressed (shift, alt, etc.)
    static uint16_t controlPressed;

    // queue of scan codes to process
    static constexpr unsigned int SCAN_CODE_QUEUE_SIZE = 32;
    static uint8_t scanCodeQueue[SCAN_CODE_QUEUE_SIZE];
    static unsigned int scanCodeQHead;
    static unsigned int scanCodeQTail;

    // queue of keys
    static constexpr unsigned int KEY_QUEUE_SIZE = 32;
    static uint16_t keyQueue[KEY_QUEUE_SIZE];
    static unsigned int keyQHead;
    static unsigned int keyQTail;

    static void keyRelease(uint16_t key);

    static void keyPress(uint16_t key);

    static char shift(char ch);

    static void updateLights();

    static void addKey(uint16_t);
};

} // namespace os

#endif // KEYBOARD_H_
