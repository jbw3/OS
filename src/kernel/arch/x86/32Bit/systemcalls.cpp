#include "isr.h"
#include "screen.h"
#include "systemcalls.h"

void systemCallHandler(const registers* regs)
{
    screen << "system call " << regs->eax << '\n';
}
