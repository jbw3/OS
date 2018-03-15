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
    void logTrace(const char* str, Ts... ts)
    {
        logMessage<eTrace>("TRACE: ", str, ts...);
    }

    template<typename... Ts>
    void logDebug(const char* str, Ts... ts)
    {
        logMessage<eDebug>("DEBUG: ", str, ts...);
    }

    template<typename... Ts>
    void logInfo(const char* str, Ts... ts)
    {
        logMessage<eInfo>("INFO: ", str, ts...);
    }

    template<typename... Ts>
    void logWarning(const char* str, Ts... ts)
    {
        logMessage<eWarning>("WARNING: ", str, ts...);
    }

    template<typename... Ts>
    void logError(const char* str, Ts... ts)
    {
        logMessage<eError>("ERROR: ", str, ts...);
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
    void logMessage([[maybe_unused]] const char* header, [[maybe_unused]] const char* str, Ts... ts)
    {
        if constexpr (LEVEL <= logLevel)
        {
            write(header);
            log(str, ts...);
        }
    }

    // this is the base-case for the recursive log() function
    void log(const char* str)
    {
        write(str);
        write('\n');
    }

    template<typename T, typename... Ts>
    void log(const char* str, T t, Ts... ts)
    {
        for (const char* ptr = str; *ptr != '\0'; ++ptr)
        {
            // check for format field
            if (ptr[0] == '{' && ptr[1] == '}')
            {
                size_t fmtSize = ptr - str;

                // write the format up to the field
                if (fmtSize > 0)
                {
                    write(str, ptr - str);
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
        log(str);
    }
};

extern Logger klog;

#endif // LOGGER_H_
