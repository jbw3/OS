#include "stream.h"

ssize_t Stream::write(const uint8_t* buff, size_t nbyte, bool block)
{
    ssize_t rv = -1;

    if (block)
    {
        size_t num = nbyte;
        do
        {
            rv = write(buff, num);
            buff += rv;
            num -= rv;

            /// @todo if this is being called from a process, we should probably yield here

            /// @todo Need to check if size_t can be cast to ssize_t
        } while (rv >= 0 && rv < static_cast<ssize_t>(nbyte));
    }
    else
    {
        rv = write(buff, nbyte);
    }

    return rv;
}
