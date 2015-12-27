#include "isr.h"
#include "screen.h"

extern "C"
void isrHandler(struct registers regs)
{
    screen.write("Interrupt\n");
}
