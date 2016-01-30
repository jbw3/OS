#include "isr.h"
#include "screen.h"

namespace
{

const char* EXCEPTION_MESSAGES[32] =
{
    "Division By Zero",
    "Debug",
    "Non-Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 Floating-Point",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point",
    "Virtualizatin",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Security",
    "Reserved",
};

/**
 * @brief Array of function pointers for ISR handlers
 */
isrHandlerPtr isrFunctions[32] =
{
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
};

}

void registerIsrHandler(uint8_t isr, isrHandlerPtr handler)
{
    isrFunctions[isr] = handler;
}

void unregisterIsrHandler(uint8_t isr)
{
    isrFunctions[isr] = nullptr;
}

/**
 * @brief Called from assembly to handle ISR
 */
extern "C"
void isrHandler(const struct registers* regs)
{
    isrHandlerPtr handler = nullptr;
    if (regs->intNo < 32)
    {
        handler = isrFunctions[regs->intNo];
    }

    // if an interrupt handler is registered, call it
    if (handler != nullptr)
    {
        handler(regs);
    }
    else // print an error message and hang
    {
        os::Screen::EColor bgColor = screen.getBackgroundColor();
        os::Screen::EColor fgColor = screen.getForegroundColor();

        screen.setBackgroundColor(os::Screen::EColor::eRed);
        screen.setForegroundColor(os::Screen::EColor::eWhite);

        if (regs->intNo < 32)
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

        while (true);
    }

}
