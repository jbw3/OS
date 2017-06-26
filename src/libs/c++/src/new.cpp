#include <new>

void operator delete(void*, std::size_t)
{
    /// @todo Implement this. We need this right now
    /// for GCC to link.
    while (true);
}
