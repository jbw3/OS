#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

uint8_t inb(uint16_t port);

void outb(uint16_t port, uint8_t value);

extern const void* getStackPointer();

extern const void* getStackStart();

extern const void* getStackEnd();

extern uint32_t getStackOffset();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SYSTEM_H_
