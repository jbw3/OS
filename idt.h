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

#ifdef __cplusplus
} // extern "C"
#endif

#endif // IDT_H_
