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
    void logTrace([[maybe_unused]] const char* str, Ts... ts)
    {
        if constexpr (LEVEL <= eTrace)
        {
            write("TRACE: ");
            log(str, ts...);
        }
    }

    template<typename... Ts>
    void logDebug([[maybe_unused]] const char* str, Ts... ts)
    {
        if constexpr (LEVEL <= eDebug)
        {
            write("DEBUG: ");
            log(str, ts...);
        }
    }

    template<typename... Ts>
    void logInfo([[maybe_unused]] const char* str, Ts... ts)
    {
        if constexpr (LEVEL <= eInfo)
        {
            write("INFO: ");
            log(str, ts...);
        }
    }

private:
    Stream* stream;

    void write(const char* msg, size_t len);

    void write(const char* str)
    {
        write(str, strlen(str));
    }

    // this is the base-case for the recursive log() function
    void log(const char* str)
    {
        write(str);
    }

    template<typename T, typename... Ts>
    void log(const char* str, T t, Ts... ts)
    {
        for (const char* ptr = str; *ptr != '\0'; ++ptr)
        {
            // check for format field
            if (ptr[0] == '{' && ptr[1] == '}')
            {
                // write the format up to the field
                write(str, ptr - str);

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
        write(str);
    }
};

extern Logger klog;

#endif // LOGGER_H_
