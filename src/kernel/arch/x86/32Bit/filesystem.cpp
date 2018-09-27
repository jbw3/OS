#include "filesystem.h"
#include "stream.h"
#include "streamtable.h"

FileSystem* FileSystem::rootFileSystem = nullptr;

FileSystem* FileSystem::getRootFileSystem()
{
    return rootFileSystem;
}

void FileSystem::setRootFileSystem(FileSystem* fs)
{
    rootFileSystem = fs;
}

int FileSystem::open(const char* path)
{
    Stream* stream = openStream(path);

    // open the stream
    int streamIdx = -1;
    if (stream != nullptr)
    {
        streamIdx = streamTable.addStream(stream);

        // if there was an error, close the stream
        if (streamIdx < 0)
        {
            stream->close();
        }
    }

    return streamIdx;
}

void FileSystem::close(int streamIdx)
{
    Stream* stream = streamTable.getStream(streamIdx);
    if (stream != nullptr)
    {
        bool removed = streamTable.removeStreamReference(streamIdx);
        if (removed)
        {
            stream->close();
        }
    }
}
