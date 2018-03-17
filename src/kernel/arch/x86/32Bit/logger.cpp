#include <string.h>

#include "logger.h"
#include "stream.h"

/// @todo Find a better way to reference this
#include "../../../../libs/c/src/stringutils.h"

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

void Logger::write(bool b)
{
    if (b)
    {
        write("true", 4);
    }
    else
    {
        write("false", 5);
    }
}

void Logger::write(int num)
{
    char buff[MAX_INT_CHARS<int>];
    size_t len = signedIntToString(num, buff, 10, false);
    write(buff, len);
}

void Logger::write(unsigned int num)
{
    char buff[MAX_INT_CHARS<unsigned int>];
    size_t len = unsignedIntToString(num, buff, 10, false);
    write(buff, len);
}

// create kernel logger instance
Logger klog;
