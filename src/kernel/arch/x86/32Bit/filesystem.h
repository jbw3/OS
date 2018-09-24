#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

class Stream;

class FileSystem
{
public:
    int open(const char* path);

    void close(int fd);

protected:
    virtual Stream* openStream(const char* path) = 0;
};

#endif // FILE_SYSTEM_H_
