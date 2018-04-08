#include <stdint.h>

#include "stream.h"
#include "userlogger.h"

UserLogger::UserLogger() :
    Logger(/*flushAfterMessage=*/ true)
{
    streamsSize = 0;
}

void UserLogger::addStream(Stream* stream)
{
    if (streamsSize < MAX_STREAMS_SIZE)
    {
        streams[streamsSize++] = stream;
    }
}

void UserLogger::flush(const char* buff, size_t len)
{
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(buff);

    for (size_t i = 0; i < streamsSize; ++i)
    {
        streams[i]->write(ptr, len, /*block=*/ true);
    }
}

// create user logger instance
UserLogger ulog;
