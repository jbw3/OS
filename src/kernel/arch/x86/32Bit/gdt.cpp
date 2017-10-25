#include "gdt.h"
#include "string.h"

// this function is defined in assembly
extern "C"
void loadGdt(uint32_t gdtAddr);

// this function is defined in assembly
extern "C"
void loadTss(uint16_t tssIndex);

constexpr int NUM_GDT_ENTRIES = 6;
struct GdtEntry gdtEntries[NUM_GDT_ENTRIES];
GdtPtr gdtPtr;
TssEntry tssEntry;

static void gdtSetGate(int32_t idx, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    GdtEntry* entry = gdtEntries + idx;

    entry->baseLow     = base & 0xFFFF;
    entry->baseMiddle  = (base >> 16) & 0xFF;
    entry->baseHigh    = (base >> 24) & 0xFF;

    entry->limitLow    = limit & 0xFFFF;
    entry->granularity = (limit >> 16) & 0x0F;

    entry->granularity |= gran & 0xF0;
    entry->access      = access;
}

static void setTss(int32_t idx, uint16_t ss0)
{
    // initialize TSS to 0
    memset(&tssEntry, 0, sizeof(TssEntry));

    // set stack segment
    tssEntry.ss0 = ss0;

    // compute base and limit
    uint32_t base = reinterpret_cast<uint32_t>(&tssEntry);
    uint32_t limit = base + sizeof(TssEntry);

    // add TSS to the GDT
    gdtSetGate(idx, base, limit, 0xE9, 0x00);
}

void initGdt()
{
    // set up the GDT pointer and limit
    gdtPtr.limit = (sizeof(GdtEntry) * NUM_GDT_ENTRIES) - 1;
    gdtPtr.base = reinterpret_cast<uint32_t>(&gdtEntries);

    gdtSetGate(0, 0, 0, 0, 0);                // null segment
    gdtSetGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // kernel code segment
    gdtSetGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // kernel data segment
    gdtSetGate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // user mode code segment
    gdtSetGate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // user mode data segment
    setTss(5, 0x10);                          // TSS

    // load the new GDT
    loadGdt(reinterpret_cast<uint32_t>(&gdtPtr));

    // load the TSS
    loadTss(0x28);
}

void setKernelStack(uint32_t stackAddr)
{
    tssEntry.esp0 = stackAddr;
}
