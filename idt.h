#ifndef IDT_H_
#define IDT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief A struct describing an interrupt gate
 */
struct IdtEntry
{
    uint16_t baseLo; ///< the lower 16 bits of the address to jump to when this interrupt fires
    uint16_t sel;    ///< kernel segment selector
    uint8_t always0; ///< this must always be zero
    uint8_t flags;   ///< more flags
    uint16_t baseHi; ///< the upper 16 bits of the address to jump to
} __attribute__((packed));

/**
 * @brief A struct describing a pointer to an array of interrupt
 * handlers. This is in a format suitable for giving to 'lidt'
 */
struct IdtPtr
{
    uint16_t limit;
    uint32_t base; ///< the address of the first element in our IdtEntry array
} __attribute__((packed));

void initIdt();

// the following extern directives let us access the address of the assembly ISR handlers
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // IDT_H_
