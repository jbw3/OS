#include "gdt.h"

// this function is defined in assembly
extern void gdtFlush(uint32_t);

// internal function prototypes
static void gdtSetGate(int32_t idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);

const int NUM_GDT_ENTRIES = 5;
struct GdtEntry gdtEntries[5];
struct GdtPtr gdtPtr;

void initGdt()
{
    // set up the GDT pointer and limit
    gdtPtr.limit = (sizeof(struct GdtEntry) * NUM_GDT_ENTRIES) - 1;
    gdtPtr.base = (uint32_t)&gdtEntries;

    gdtSetGate(0, 0, 0, 0, 0);                // null segment
    gdtSetGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // kernel code segment
    gdtSetGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // kernel data segment
    gdtSetGate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // user mode code segment
    gdtSetGate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // user mode data segment

    // flush out the old GDT and intall the new one
    gdtFlush((uint32_t)&gdtPtr);
}

static void gdtSetGate(int32_t idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    struct GdtEntry* entry = gdtEntries + idx;

    (*entry).baseLow     = base & 0xFFFF;
    (*entry).baseMiddle  = (base >> 16) & 0xFF;
    (*entry).baseHigh    = (base >> 24) & 0xFF;

    (*entry).limitLow    = limit & 0xFFFF;
    (*entry).granularity = (limit >> 16) & 0x0F;

    (*entry).granularity |= gran & 0xF0;
    (*entry).access      = access;
}
