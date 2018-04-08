#include <string.h>

#include "kernellogger.h"
#include "stream.h"

KernelLogger::KernelLogger()
{
    stream = nullptr;
}

void KernelLogger::setStream(Stream* streamPtr)
{
    stream = streamPtr;
}

void KernelLogger::flush(const char* buff, size_t len)
{
    if (stream != nullptr)
    {
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(buff);
        stream->write(ptr, len, /*block=*/ true);
    }
}

void KernelLogger::writeHeader(const char* levelStr, const char* tag)
{
    write(levelStr);
    write(": ", 2);
    write(tag);
    write(": ", 2);
}

// create kernel logger instance
KernelLogger klog;
