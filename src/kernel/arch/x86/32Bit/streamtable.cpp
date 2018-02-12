#include "string.h"

#include "streamtable.h"

StreamTable::StreamTable()
{
    // init all stream pointers to null
    memset(streams, 0, MAX_NUM_STREAMS);
}

int StreamTable::addStream(Stream* stream)
{
    for (int i = 0; i < MAX_NUM_STREAMS; ++i)
    {
        if (streams[i] == nullptr)
        {
            streams[i] = stream;
            return i;
        }
    }

    return -1;
}

void StreamTable::removeStream(int streamIdx)
{
    if (streamIdx >= 0 && streamIdx < MAX_NUM_STREAMS)
    {
        streams[streamIdx] = nullptr;
    }
}

Stream* StreamTable::getStream(int streamIdx) const
{
    if (streamIdx >= 0 && streamIdx < MAX_NUM_STREAMS)
    {
        return streams[streamIdx];
    }

    return nullptr;
}

// create StreamTable instance
StreamTable streamTable;
