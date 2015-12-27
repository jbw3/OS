#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

void outb(uint16_t port, uint8_t value);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SYSTEM_H_
