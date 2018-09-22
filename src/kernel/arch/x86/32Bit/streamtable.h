#ifndef STREAM_TABLE_H_
#define STREAM_TABLE_H_

#include <stddef.h>

class Stream;

class StreamTable
{
public:
    StreamTable();

    int addStream(Stream* stream);

    void addStreamReference(int streamIdx);

    void removeStreamReference(int streamIdx);

    Stream* getStream(int streamIdx) const;

private:
    constexpr static int MAX_NUM_STREAMS = 16;

    Stream* streams[MAX_NUM_STREAMS];
    size_t streamsRefCounts[MAX_NUM_STREAMS];
};

extern StreamTable streamTable;

#endif // STREAM_TABLE_H_
