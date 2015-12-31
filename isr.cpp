#include "isr.h"
#include "screen.h"

namespace
{

/// @todo Fill in "Reserved" strings
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

/**
 * @brief Called from assembly to handle ISR
 */
extern "C"
void isrHandler(const struct registers* regs)
{
    os::Screen::EColor bgColor = screen.getBackgroundColor();
    os::Screen::EColor fgColor = screen.getForegroundColor();

    screen.setBackgroundColor(os::Screen::EColor::eRed);
    screen.setForegroundColor(os::Screen::EColor::eWhite);

    if (regs->intNo >= 0 && regs->intNo < 32)
    {
        screen << EXCEPTION_MESSAGES[regs->intNo] << " Exception\n";
    }
    else
    {
        screen << "Unknown exception number: " << regs->intNo << '\n';
    }

    screen << "Error code: " << regs->errCode << '\n';

    screen.setBackgroundColor(bgColor);
    screen.setForegroundColor(fgColor);
}
