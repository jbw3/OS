#include <string.h>

#include "logger.h"
#include "stream.h"

Logger::FormatOptions::FormatOptions()
{
    reset();
}

void Logger::FormatOptions::reset()
{
    base = 10;
    uppercase = false;
}

Logger::Logger()
{
    stream = nullptr;
}

void Logger::setStream(Stream* streamPtr)
{
    stream = streamPtr;
}

void Logger::writeHeader(const char* levelStr, const char* tag)
{
    write(levelStr);
    write(": ", 2);
    write(tag);
    write(": ", 2);
}

bool Logger::writeFormatAndParseOptions(const char* format, const char*& nextFormat)
{
    bool foundOptions = false;
    nextFormat = nullptr;

    // find start of format
    const char* fmtStart = strchr(format, '{');
    if (fmtStart != nullptr)
    {
        // find end of format
        const char* fmtEnd = strchr(fmtStart, '}');
        if (fmtEnd != nullptr)
        {
            // parse format
            bool parsingOk = parseOptions(fmtStart + 1, fmtEnd);
            if (parsingOk)
            {
                foundOptions = true;
                nextFormat = fmtEnd + 1;

                // write the format up to the field
                size_t size = fmtStart - format;
                if (size > 0)
                {
                    write(format, size);
                }
            }
        }
    }

    return foundOptions;
}

bool Logger::parseOptions(const char* fmtStart, const char* fmtEnd)
{
    bool ok = true;

    // reset format
    fmtOptions.reset();

    if (fmtStart != fmtEnd)
    {
        switch (*fmtStart)
        {
        case 'b':
            fmtOptions.base = 2;
            break;

        case 'o':
            fmtOptions.base = 8;
            break;

        case 'X':
            fmtOptions.uppercase = true;
            [[fallthrough]];
        case 'x':
            fmtOptions.base = 16;
            break;

        default:
            ok = false;
            break;
        }
    }

    return ok;
}

void Logger::write(const char* msg, size_t len)
{
    if (stream != nullptr)
    {
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(msg);

        stream->write(ptr, len);
    }
}

void Logger::write(const char* str)
{
    write(str, strlen(str));
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
