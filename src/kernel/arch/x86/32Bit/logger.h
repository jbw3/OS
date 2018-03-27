#ifndef LOGGER_H_
#define LOGGER_H_

#include <stddef.h>
#include <string.h>

/// @todo Find a better way to reference this
#include "../../../../libs/c/src/stringutils.h"

class Stream;

class Logger
{
public:
    /**
     * @brief The logging level.
     */
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
    /**
     * @brief Used to fail a static_assert().
     */
    template<typename T>
    struct dependent_false : std::false_type
    {};

    /**
     * @brief Contains format options.
     */
    struct FormatOptions
    {
        int base;
        bool uppercase;
        size_t width;
        char fill;
        enum EAlignment
        {
            eLeft,
            eRight,
            eCenter,
        } alignment;

        FormatOptions();

        void reset();
    } fmtOptions;

    Stream* stream;

    /**
     * @brief Write the message header.
     * @param levelStr The debug level string.
     * @param tag The message tag.
     */
    void writeHeader(const char* levelStr, const char* tag);

    bool writeFormatAndParseOptions(const char* format, const char*& nextFormat, FormatOptions& options);

    /**
     * @brief Parse the format options.
     * @param fmtStart Points to the first character in the format.
     * @param fmtEnd Points to one past the last character in the format.
     * @param [out] options The format options parsed from the string.
     * @return true if parsing was successful.
     * @return false if there was an error during parsing.
     */
    bool parseOptions(const char* fmtStart, const char* fmtEnd, FormatOptions& options);

    void write(const char* msg, size_t len);

    void write(const char* str);

    void write(bool b);

    void write(char c)
    {
        write(&c, 1);
    }

    void write(signed char num);

    void write(short num);

    void write(int num);

    void write(long num);

    void write(long long num);

    void write(unsigned char num);

    void write(unsigned short num);

    void write(unsigned int num);

    void write(unsigned long num);

    void write(unsigned long long num);

    void write(const void* ptr);

    template<typename T>
    void writeInt([[maybe_unused]] T value)
    {
        static_assert(std::is_integral_v<T>, "writeInt() only writes integral values.");

        char buff[MAX_INT_CHARS<T>];
        size_t size = intToString(value, buff, fmtOptions.base, fmtOptions.uppercase);
        write(buff, size);
    }

    template<ELevel logLevel, typename... Ts>
    void logMessage([[maybe_unused]] const char* levelStr, [[maybe_unused]] const char* tag, [[maybe_unused]] const char* format, Ts... ts)
    {
        if constexpr (LEVEL <= logLevel)
        {
            writeHeader(levelStr, tag);
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
        const char* nextFormat = nullptr;
        FormatOptions options;
        bool foundOptions = writeFormatAndParseOptions(format, nextFormat, options);
        if (foundOptions)
        {
            // set the options
            fmtOptions = options;

            // write the field
            write(t);

            // reset the options to default values
            fmtOptions.reset();

            // recursive call with the remaining format string and arguments
            log(nextFormat, ts...);
        }
        else
        {
            // if we get here, there were no fields in the format string, so
            // just print the string
            log(format);
        }
    }
};

extern Logger klog;

#endif // LOGGER_H_
