#include "filesystem.h"
#include "stream.h"
#include "streamtable.h"

int FileSystem::open(const char* path)
{
    Stream* stream = openStream(path);

    // open the stream
    int fd = -1;
    if (stream != nullptr)
    {
        fd = streamTable.addStream(stream);

        // if there was an error, close the stream
        if (fd < 0)
        {
            stream->close();
        }
    }

    return fd;
}

void FileSystem::close(int fd)
{
    Stream* stream = streamTable.getStream(fd);
    if (stream != nullptr)
    {
        stream->close();

        streamTable.removeStream(fd);
    }
}
