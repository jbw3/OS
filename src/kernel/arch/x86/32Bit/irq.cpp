#include "idt.h"
#include "irq.h"
#include "system.h"

const int IRQ_START_NUM = 32;
const int NUM_IRQ_FUNCTIONS = 16;

/**
 * @brief Array of function pointers for IRQ handlers
 */
irqHandlerPtr irqFunctions[16] =
{
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
};

namespace
{

/**
 * @brief Remap IRQ0 to IRQ15 to IDT entries 32 to 47
 */
void remapIrq()
{
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

}

void initIrq()
{
    remapIrq();

    idtSetGate(IRQ_START_NUM +  0, (uint32_t)irq0 , 0x08, 0x8E);
    idtSetGate(IRQ_START_NUM +  1, (uint32_t)irq1 , 0x08, 0x8E);
    idtSetGate(IRQ_START_NUM +  2, (uint32_t)irq2 , 0x08, 0x8E);
    idtSetGate(IRQ_START_NUM +  3, (uint32_t)irq3 , 0x08, 0x8E);
    idtSetGate(IRQ_START_NUM +  4, (uint32_t)irq4 , 0x08, 0x8E);
    idtSetGate(IRQ_START_NUM +  5, (uint32_t)irq5 , 0x08, 0x8E);
    idtSetGate(IRQ_START_NUM +  6, (uint32_t)irq6 , 0x08, 0x8E);
    idtSetGate(IRQ_START_NUM +  7, (uint32_t)irq7 , 0x08, 0x8E);
    idtSetGate(IRQ_START_NUM +  8, (uint32_t)irq8 , 0x08, 0x8E);
    idtSetGate(IRQ_START_NUM +  9, (uint32_t)irq9 , 0x08, 0x8E);
    idtSetGate(IRQ_START_NUM + 10, (uint32_t)irq10, 0x08, 0x8E);
    idtSetGate(IRQ_START_NUM + 11, (uint32_t)irq11, 0x08, 0x8E);
    idtSetGate(IRQ_START_NUM + 12, (uint32_t)irq12, 0x08, 0x8E);
    idtSetGate(IRQ_START_NUM + 13, (uint32_t)irq13, 0x08, 0x8E);
    idtSetGate(IRQ_START_NUM + 14, (uint32_t)irq14, 0x08, 0x8E);
    idtSetGate(IRQ_START_NUM + 15, (uint32_t)irq15, 0x08, 0x8E);
}

void registerIrqHandler(uint8_t irq, irqHandlerPtr handler)
{
    irqFunctions[irq] = handler;
}

void unregisterIrqHandler(uint8_t irq)
{
    irqFunctions[irq] = nullptr;
}

/**
 * @brief Called from assembly to handle IRQ
 */
extern "C"
void irqHandler(const struct registers* regs)
{
    // function pointer
    irqHandlerPtr handler = irqFunctions[regs->intNo - IRQ_START_NUM];

    // if a handler is registered, execute it
    if (handler != nullptr)
    {
        handler(regs);
    }

    // if the IDT entry is greater than or equal to IRQ8,
    // send an EOI to the slave interrupt controller
    if (regs->intNo >= IRQ_START_NUM + 8)
    {
        outb(0xA0, 0x20);
    }

    // always send an EOI to the master interrupt controller
    outb(0x20, 0x20);
}
