#include "timer.h"
#include "screen.h"
#include "system.h"

namespace os
{

uint64_t Timer::ticks = 0;

void Timer::init(unsigned int freq)
{
    ticks = 0;

    // register interrupt
    installIrqHandler(IRQ_TIMER, interruptHandler);

    // the value we send to the PIT is the value to divide it's
    // 1,193,180 Hz input clock by
    uint16_t divisor = PIT_FREQUENCY / freq;

    // set the command byte
    outb(0x43, 0x36);

    // set the divisor
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);
}

void Timer::interruptHandler(const struct registers* regs)
{
    ++ticks;
}

} // namespace os
