#ifndef GDT_H_
#define GDT_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Contains the value of one GDT entry
 */
struct GdtEntry
{
    uint16_t limitLow;   ///< the lower 16 bits of the limit
    uint16_t baseLow;    ///< the lower 16 bits of the base
    uint8_t baseMiddle;  ///< the next 8 bits of the base
    uint8_t access;      ///< access flags, determines what ring this segment can be used in
    uint8_t granularity;
    uint8_t baseHigh;    ///< the last 8 bits of the base
} __attribute__((packed));

/**
 * @brief A TSS entry in the GDT.
 */
struct TssEntry
{
    uint16_t prevTss;
    uint16_t reserved0;
    uint32_t esp0;      ///< the stack pointer to load when changing to kernel mode
    uint16_t ss0;       ///< the stack segment to load when changing to kernel mode
    uint16_t reserved1;
    uint32_t esp1;
    uint16_t ss1;
    uint16_t reserved2;
    uint32_t esp2;
    uint16_t ss2;
    uint16_t reserved3;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint16_t es;
    uint16_t reserved4;
    uint16_t cs;
    uint16_t reserved5;
    uint16_t ss;
    uint16_t reserved6;
    uint16_t ds;
    uint16_t reserved7;
    uint16_t fs;
    uint16_t reserved8;
    uint16_t gs;
    uint16_t reserved9;
    uint16_t ldtSegmentSelector;
    uint16_t reserved10;
    uint16_t trap;
    uint16_t ioMapBase;
} __attribute__((packed));

struct GdtPtr
{
    uint16_t limit; ///< the upper 16 bits of all selector limits
    uint32_t base;  ///< the address of the first GdtEntry struct
} __attribute__((packed));

/**
 * @brief Initialize the Global Descriptor Table
 */
void initGdt();

/**
 * @brief Set the kernel stack in the TSS.
 */
void setKernelStack(uint32_t stackAddr);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // GDT_H_
