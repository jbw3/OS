#include "system.h"

uint8_t inb(uint16_t port)
{
    uint8_t rv;
    __asm volatile ("inb %1, %0" : "=a" (rv) : "dN" (port));
    return rv;
}

void outb(uint16_t port, uint8_t value)
{
    __asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}
