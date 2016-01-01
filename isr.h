#ifndef ISR_H_
#define ISR_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

struct registers
{
    uint32_t ds;      ///< data segment selector

    uint32_t edi;     ///< pushed by pusha
    uint32_t esi;     ///< pushed by pusha
    uint32_t ebp;     ///< pushed by pusha
    uint32_t uselessValue; ///< pushed by pusha
    uint32_t ebx;     ///< pushed by pusha
    uint32_t edx;     ///< pushed by pusha
    uint32_t ecx;     ///< pushed by pusha
    uint32_t eax;     ///< pushed by pusha

    uint32_t intNo;   ///< interrupt number
    uint32_t errCode; ///< error code (if applicable)

    uint32_t eip;     ///< pushed by the processor automatically
    uint32_t cs;      ///< pushed by the processor automatically
    uint32_t eflags;  ///< pushed by the processor automatically
    uint32_t esp;     ///< pushed by the processor automatically
    uint32_t ss;      ///< pushed by the processor automatically
};

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

#endif // ISR_H_
