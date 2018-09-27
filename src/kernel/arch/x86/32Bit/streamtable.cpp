#include "string.h"

#include "streamtable.h"

StreamTable::StreamTable()
{
    // init all stream pointers to null
    memset(streams, 0, sizeof(streams));

    // init all stream ref counts to 0
    memset(streamsRefCounts, 0, sizeof(streamsRefCounts));
}

int StreamTable::addStream(Stream* stream)
{
    for (int i = 0; i < MAX_NUM_STREAMS; ++i)
    {
        if (streams[i] == nullptr)
        {
            streams[i] = stream;
            ++streamsRefCounts[i];
            return i;
        }
    }

    return -1;
}

void StreamTable::addStreamReference(int streamIdx)
{
    if (streamIdx >= 0 && streamIdx < MAX_NUM_STREAMS && streamsRefCounts[streamIdx] > 0)
    {
        ++streamsRefCounts[streamIdx];
    }
}

bool StreamTable::removeStreamReference(int streamIdx)
{
    bool removed = false;

    if (streamIdx >= 0 && streamIdx < MAX_NUM_STREAMS && streamsRefCounts[streamIdx] > 0)
    {
        // decrement the stream ref count
        --streamsRefCounts[streamIdx];

        // if the ref count is now zero, remove the stream
        if (streamsRefCounts[streamIdx] == 0)
        {
            streams[streamIdx] = nullptr;
            removed = true;
        }
    }

    return removed;
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
