#include <string.h>
#include "filesystem.h"
#include "rootfilesystem.h"
#include "stream.h"
#include "streamtable.h"

RootFileSystem::RootFileSystem()
{
    numFileSystems = 0;
    memset(fileSystems, 0, sizeof(fileSystems));
}

void RootFileSystem::addFileSystem(FileSystem* fileSystem)
{
    if (numFileSystems < MAX_NUM_FILE_SYSTEMS)
    {
        fileSystems[numFileSystems++] = fileSystem;
    }
}

int RootFileSystem::open(const char* path)
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

void RootFileSystem::close(int streamIdx)
{
    streamTable.removeStreamReference(streamIdx);
}

Stream* RootFileSystem::openStream(const char* path)
{
    Stream* stream = nullptr;

    /// @todo looping through the file systems trying to open the file in
    /// each one is not a good long-term solution
    for (FileSystem* fileSystem : fileSystems)
    {
        if (fileSystem != nullptr)
        {
            stream = fileSystem->openStream(path);
            if (stream != nullptr)
            {
                break;
            }
        }
    }

    return stream;
}

// init root file system
RootFileSystem rootFileSystem;
