#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>

#include "irq.h"

#define PIT_FREQUENCY 1193180

namespace os
{

class Timer
{
public:
    static void init(unsigned int freq);

    static void interruptHandler(const struct registers* regs);

private:
    static uint64_t ticks;
};

} // namespace os

#endif // TIMER_H_
