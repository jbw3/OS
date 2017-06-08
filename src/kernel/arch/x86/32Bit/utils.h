#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>

constexpr uint32_t align(uint32_t value, unsigned int alignment)
{
    uint32_t mask = alignment - 1;
    if ( (value & mask) != 0 )
    {
        value = (value & ~mask) + alignment;
    }

    return value;
}

#endif // UTILS_H_
