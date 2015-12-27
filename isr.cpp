#include "isr.h"
#include "screen.h"

namespace
{

const char* EXCEPTION_MESSAGES[] =
{
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
};

}

extern "C"
void isrHandler(struct registers regs)
{
    os::Screen::EColor bgColor = screen.getBackgroundColor();
    os::Screen::EColor fgColor = screen.getForegroundColor();

    screen.setBackgroundColor(os::Screen::EColor::eRed);
    screen.setForegroundColor(os::Screen::EColor::eWhite);

    if (regs.intNo >= 0 && regs.intNo < 32)
    {
        screen.write(EXCEPTION_MESSAGES[regs.intNo]);
        screen.write(" Exception\n");
    }
    else
    {
        screen.write("Unknown exception number\n");
    }

    screen.setBackgroundColor(bgColor);
    screen.setForegroundColor(fgColor);
}
