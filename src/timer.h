#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>

#include "irq.h"

#define PIT_FREQUENCY 1'193'180

namespace os
{

class Timer
{
public:
    /**
     * @brief Initialize the timer with the frequency at which to interrupt
     * @param freq the timer frequency in Hz, this cannot be less than 19
     */
    static void init(unsigned int freq);

    static uint64_t getTicks();

    static void interruptHandler(const struct registers* regs);

private:
    static uint64_t ticks;
};

} // namespace os

#endif // TIMER_H_
