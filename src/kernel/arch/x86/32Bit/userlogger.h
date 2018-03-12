#ifndef USER_LOGGER_H_
#define USER_LOGGER_H_

#include "logger.h"

class Stream;

/**
 * @brief Provides capability for logging messages to the user.
 */
class UserLogger : public Logger
{
public:
    UserLogger();

    void addStream(Stream* stream);

protected:
    void flush(const char* buff, size_t len) override;

private:
    static constexpr size_t MAX_STREAMS_SIZE = 4;
    Stream* streams[MAX_STREAMS_SIZE];
    size_t streamsSize;
};

extern UserLogger ulog;

#endif // USER_LOGGER_H_
