#ifndef LOGGER_H_
#define LOGGER_H_

#include <stddef.h>
#include <string.h>

class Stream;

class Logger
{
public:
    enum ELevel
    {
        eTrace,
        eDebug,
        eInfo,
        eWarning,
        eError,
        eOff
    };

    static constexpr ELevel LEVEL = eInfo;

    Logger();

    void setStream(Stream* streamPtr);

    template<typename... Ts>
    void logTrace(const char* tag, const char* format, Ts... ts)
    {
        logMessage<eTrace>("TRACE", tag, format, ts...);
    }

    template<typename... Ts>
    void logDebug(const char* tag, const char* format, Ts... ts)
    {
        logMessage<eDebug>("DEBUG", tag, format, ts...);
    }

    template<typename... Ts>
    void logInfo(const char* tag, const char* format, Ts... ts)
    {
        logMessage<eInfo>("INFO", tag, format, ts...);
    }

    template<typename... Ts>
    void logWarning(const char* tag, const char* format, Ts... ts)
    {
        logMessage<eWarning>("WARNING", tag, format, ts...);
    }

    template<typename... Ts>
    void logError(const char* tag, const char* format, Ts... ts)
    {
        logMessage<eError>("ERROR", tag, format, ts...);
    }

private:
    Stream* stream;

    void write(const char* msg, size_t len);

    void write(const char* str)
    {
        write(str, strlen(str));
    }

    void write(char ch)
    {
        write(&ch, 1);
    }

    template<ELevel logLevel, typename... Ts>
    void logMessage([[maybe_unused]] const char* levelStr, [[maybe_unused]] const char* tag, [[maybe_unused]] const char* format, Ts... ts)
    {
        if constexpr (LEVEL <= logLevel)
        {
            write(levelStr);
            write(": ");
            write(tag);
            write(": ");
            log(format, ts...);
        }
    }

    // this is the base-case for the recursive log() function
    void log(const char* format)
    {
        write(format);
        write('\n');
    }

    template<typename T, typename... Ts>
    void log(const char* format, T t, Ts... ts)
    {
        for (const char* ptr = format; *ptr != '\0'; ++ptr)
        {
            // check for format field
            if (ptr[0] == '{' && ptr[1] == '}')
            {
                size_t fmtSize = ptr - format;

                // write the format up to the field
                if (fmtSize > 0)
                {
                    write(format, ptr - format);
                }

                // write the field
                write(t);

                // recursive call with the remaining format string and arguments
                log(ptr + 2, ts...);

                // the recursive call will print the rest of the string, so
                // there is nothing left to do
                return;
            }
        }

        // if we get here, there were no fields in the format string, so
        // just print the string
        log(format);
    }
};

extern Logger klog;

#endif // LOGGER_H_
