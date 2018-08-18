#ifndef UTILS_H_
#define UTILS_H_

#include <stdint.h>

constexpr uintptr_t align(uintptr_t value, unsigned int alignment, bool alignNext = true)
{
    uintptr_t mask = alignment - 1;
    if ( (value & mask) != 0 )
    {
        value &= ~mask;
        if (alignNext)
        {
            value += alignment;
        }
    }

    return value;
}

constexpr void* align(const void* value, unsigned int alignment, bool alignNext = true)
{
    uintptr_t ptr = align(reinterpret_cast<uintptr_t>(value), alignment, alignNext);
    return reinterpret_cast<void*>(ptr);
}

#endif // UTILS_H_
