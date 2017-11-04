#ifndef IRQ_H_
#define IRQ_H_

#include "isr.h"

#define IRQ0   0
#define IRQ1   1
#define IRQ2   2
#define IRQ3   3
#define IRQ4   4
#define IRQ5   5
#define IRQ6   6
#define IRQ7   7
#define IRQ8   8
#define IRQ9   9
#define IRQ10 10
#define IRQ11 11
#define IRQ12 12
#define IRQ13 13
#define IRQ14 14
#define IRQ15 15

#define IRQ_TIMER      IRQ0
#define IRQ_KEYBOARD   IRQ1

#ifdef __cplusplus
extern "C"
{
#endif

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

typedef void (*irqHandlerPtr)(const registers*);

void initIrq();

/**
 * @brief Register a handler for an IRQ.
 */
void registerIrqHandler(uint8_t irq, irqHandlerPtr handler);

/**
 * @brief Unregister a handler for an IRQ.
 */
void unregisterIrqHandler(uint8_t irq);

/**
 * @brief Send End Of Interrupt (EOI) command to PIC.
 */
void sendPicEoi(const registers* regs);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // IRQ_H_
