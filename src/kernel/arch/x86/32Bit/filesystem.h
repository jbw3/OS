#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

class Stream;

class FileSystem
{
public:
    virtual Stream* openStream(const char* path) = 0;
};

#endif // FILE_SYSTEM_H_
