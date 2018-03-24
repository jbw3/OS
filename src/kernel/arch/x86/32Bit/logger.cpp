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

void Logger::write(signed char num)
{
    writeInt(num);
}

void Logger::write(short num)
{
    writeInt(num);
}

void Logger::write(int num)
{
    writeInt(num);
}

void Logger::write(long num)
{
    writeInt(num);
}

void Logger::write(long long num)
{
    writeInt(num);
}

void Logger::write(unsigned char num)
{
    writeInt(num);
}

void Logger::write(unsigned short num)
{
    writeInt(num);
}

void Logger::write(unsigned int num)
{
    writeInt(num);
}

void Logger::write(unsigned long num)
{
    writeInt(num);
}

void Logger::write(unsigned long long num)
{
    writeInt(num);
}

void Logger::write(const void* ptr)
{
    writeInt(reinterpret_cast<uintptr_t>(ptr));
}

// create kernel logger instance
Logger klog;
