#include <string.h>

#include "ctype.h"
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
    width = 0;
    fill = ' ';
    alignment = eLeft;
}

Logger::Logger()
{
    stream = nullptr;
    buffSize = 0;
}

void Logger::setStream(Stream* streamPtr)
{
    stream = streamPtr;
}

void Logger::flush()
{
    if (buffSize > 0 && stream != nullptr)
    {
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(buff);
        stream->write(ptr, buffSize);
    }

    buffSize = 0;
}

void Logger::writeHeader(const char* levelStr, const char* tag)
{
    write(levelStr);
    write(": ", 2);
    write(tag);
    write(": ", 2);
}

bool Logger::writeFormatAndParseOptions(const char* format, const char*& nextFormat, FormatOptions& options)
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
            bool parsingOk = parseOptions(fmtStart + 1, fmtEnd, options);
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

bool Logger::parseOptions(const char* fmtStart, const char* fmtEnd, FormatOptions& options)
{
    bool ok = true;

    for (const char* ptr = fmtStart; ptr != fmtEnd; ++ptr)
    {
        if (ptr + 1 != fmtEnd && (ptr[1] == '<' || ptr[1] == '^' || ptr[1] == '>'))
        {
            options.fill = *ptr;
        }
        else if (isdigit(*ptr))
        {
            options.width = 0;
            do
            {
                options.width *= 10;
                options.width += *ptr - '0';
                ++ptr;
            } while (ptr != fmtEnd && isdigit(*ptr));
            --ptr;
        }
        else
        {
            switch (*ptr)
            {
            case 'b':
                options.base = 2;
                break;

            case 'o':
                options.base = 8;
                break;

            case 'd':
                options.base = 10;
                break;

            case 'X':
                options.uppercase = true;
                [[fallthrough]];
            case 'x':
                options.base = 16;
                break;

            case '<':
                options.alignment = FormatOptions::eLeft;
                break;

            case '^':
                options.alignment = FormatOptions::eCenter;
                break;

            case '>':
                options.alignment = FormatOptions::eRight;
                break;

            default:
                ok = false;
                break;
            }
        }

        if (!ok)
        {
            break;
        }
    }

    return ok;
}

void Logger::buffWrite(const char* msg, size_t len)
{
    size_t totalCopied = 0;
    while (totalCopied < len)
    {
        size_t buffAvail = MAX_BUFF_SIZE - buffSize;
        size_t toCopy = len - totalCopied;
        size_t copySize = (toCopy <= buffAvail) ? toCopy : buffAvail;

        memcpy(buff + buffSize, msg + totalCopied, copySize);
        buffSize += copySize;
        totalCopied += copySize;
        if (buffSize >= MAX_BUFF_SIZE)
        {
            flush();
        }
    }
}

void Logger::buffWrite(char ch, size_t num)
{
    size_t totalCopied = 0;
    while (totalCopied < num)
    {
        size_t buffAvail = MAX_BUFF_SIZE - buffSize;
        size_t toCopy = num - totalCopied;
        size_t copySize = (toCopy <= buffAvail) ? toCopy : buffAvail;

        memset(buff + buffSize, ch, copySize);
        buffSize += copySize;
        totalCopied += copySize;
        if (buffSize >= MAX_BUFF_SIZE)
        {
            flush();
        }
    }
}

void Logger::write(const char* msg, size_t len)
{
    size_t leftPaddingChars = 0;
    size_t rightPaddingChars = 0;

    if (fmtOptions.width > len)
    {
        size_t totalPaddingChars = fmtOptions.width - len;
        switch (fmtOptions.alignment)
        {
        case FormatOptions::eLeft:
            rightPaddingChars = totalPaddingChars;
            break;

        case FormatOptions::eCenter:
            leftPaddingChars = totalPaddingChars / 2;
            rightPaddingChars = (totalPaddingChars + 1) / 2;
            break;

        case FormatOptions::eRight:
            leftPaddingChars = totalPaddingChars;
            break;
        }

        buffWrite(fmtOptions.fill, leftPaddingChars);
    }

    buffWrite(msg, len);

    buffWrite(fmtOptions.fill, rightPaddingChars);
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
