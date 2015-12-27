#ifndef DESCRIPTORTABLES_H_
#define DESCRIPTORTABLES_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief Contains the value of one GDT entry
 * @details [long description]
 *
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

struct GdtPtr
{
    uint16_t limit; ///< the upper 16 bits of all selector limits
    uint32_t base;  ///< the address of the first GdtEntry struct
} __attribute__((packed));

/**
 * @brief Initialize descriptor tables
 */
void initDescriptorTables();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // DESCRIPTORTABLES_H_
