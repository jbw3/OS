#include <string.h>

#include "logger.h"
#include "stream.h"

Logger::Logger()
{
    stream = nullptr;
}

void Logger::setStream(Stream* streamPtr)
{
    stream = streamPtr;
}

void Logger::write(const char* msg, size_t len)
{
    if (stream != nullptr)
    {
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(msg);

        stream->write(ptr, len);
    }
}

// create kernel logger instance
Logger klog;
