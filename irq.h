#ifndef IRQ_H_
#define IRQ_H_

#include "isr.h"

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

typedef void (*irqHandlerPtr)(const struct registers*);

void initIrq();

/**
 * @brief Install a handler for an IRQ
 */
void installIrqHandler(uint8_t irq, irqHandlerPtr handler);

/**
 * @brief Uninstall a handler for an IRQ
 */
void uninstallIrqHandler(uint8_t irq);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // IRQ_H_
