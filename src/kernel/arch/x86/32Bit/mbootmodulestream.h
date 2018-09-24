#ifndef MBOOT_MODULE_STREAM_H_
#define MBOOT_MODULE_STREAM_H_

#include "stream.h"

struct multiboot_mod_list;

class MBootModuleStream : public Stream
{
public:
    MBootModuleStream();

    void setModule(const multiboot_mod_list* modulePtr);

    bool canRead() const override
    {
        return true;
    }

    bool canWrite() const override
    {
        return false;
    }

    ssize_t read(uint8_t* buff, size_t nbyte) override;

    ssize_t write(const uint8_t*, size_t) override
    {
        return -1;
    }

    void flush() override
    {
    }

    void close();

    bool isOpen() const;

private:
    const multiboot_mod_list* module;
    size_t position;
};

#endif // MBOOT_MODULE_STREAM_H_
