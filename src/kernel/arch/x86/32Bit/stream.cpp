#include "stream.h"

ssize_t Stream::write(const uint8_t* buff, size_t nbyte, bool block)
{
    ssize_t rv = -1;

    if (block)
    {
        rv = write(buff, nbyte);
        /// @todo Need to check if size_t can be cast to ssize_t
        while (rv >= 0 && rv < static_cast<ssize_t>(nbyte))
        {
            /// @todo if this is being called from a process, we should probably yield here

            buff += rv;
            nbyte -= rv;
            rv = write(buff, nbyte);
        }
    }
    else
    {
        rv = write(buff, nbyte);
    }

    return rv;
}
