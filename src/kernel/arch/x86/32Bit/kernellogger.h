#ifndef KERNEL_LOGGER_H_
#define KERNEL_LOGGER_H_

#include <stddef.h>
#include <string.h>

#include "logger.h"

class Stream;

/**
 * @brief A class that provides logging capabilities for the kernel log.
 */
class KernelLogger : public Logger
{
public:
    /**
     * @brief The logging level.
     */
    enum ELevel
    {
        /// Used for verbose messages that should only be logged when debugging.
        eTrace,

        /// Used for messages that should only be logged when debugging.
        eDebug,

        /// Used for normal, informational messages.
        eInfo,

        /// Used to warn that something may have gone wrong.
        eWarning,

        /// Used to indicate that an error occurred.
        eError,

        /// Used to turn off logging.
        eOff
    };

    /// Messages at this level and above will be logged.
    static constexpr ELevel LOG_LEVEL = eInfo;

    /// Messages at this level and above will be flushed immediately after they are logged.
    static constexpr ELevel FLUSH_LEVEL = eInfo;

    /**
     * @brief Construct a new KernelLogger object.
     */
    KernelLogger();

    /**
     * @brief Set the stream that messages will be written to.
     * @param streamPtr The stream that messages will be written to.
     */
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

protected:
    void flush(const char* buff, size_t len) override;

private:
    Stream* stream;

    /**
     * @brief Write the message header.
     * @param levelStr The debug level string.
     * @param tag The message tag.
     */
    void writeHeader(const char* levelStr, const char* tag);

    template<ELevel level, typename... Ts>
    void logMessage([[maybe_unused]] const char* levelStr, [[maybe_unused]] const char* tag, [[maybe_unused]] const char* format, Ts... ts)
    {
        if constexpr (level >= LOG_LEVEL)
        {
            writeHeader(levelStr, tag);
            log(format, ts...);

            if constexpr (level >= FLUSH_LEVEL)
            {
                // flush the buffer
                Logger::flush();
            }
        }
    }
};

extern KernelLogger klog;

#endif // KERNEL_LOGGER_H_
