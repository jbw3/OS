#ifndef ROOT_FILE_SYSTEM_H_
#define ROOT_FILE_SYSTEM_H_

#include <stddef.h>

class FileSystem;
class Stream;

class RootFileSystem
{
public:
    RootFileSystem();

    void addFileSystem(FileSystem* fileSystem);

    int open(const char* path);

    void close(int streamIdx);

private:
    static constexpr size_t MAX_NUM_FILE_SYSTEMS = 4;
    FileSystem* fileSystems[MAX_NUM_FILE_SYSTEMS];
    size_t numFileSystems;

    Stream* openStream(const char* path);
};

extern RootFileSystem rootFileSystem;

#endif // ROOT_FILE_SYSTEM_H_
