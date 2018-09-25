#ifndef FILE_SYSTEM_H_
#define FILE_SYSTEM_H_

class Stream;

class FileSystem
{
public:
    static FileSystem* getRootFileSystem();

    static void setRootFileSystem(FileSystem* fs);

    int open(const char* path);

    void close(int streamIdx);

protected:
    virtual Stream* openStream(const char* path) = 0;

private:
    static FileSystem* rootFileSystem;
};

#endif // FILE_SYSTEM_H_
