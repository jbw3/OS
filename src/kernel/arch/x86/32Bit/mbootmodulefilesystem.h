#ifndef MBOOT_MODULE_FILE_SYSTEM_H_
#define MBOOT_MODULE_FILE_SYSTEM_H_

#include <stdint.h>
#include <stddef.h>
#include "filesystem.h"
#include "mbootmodulestream.h"

struct multiboot_info;

class MBootModuleFileSystem : public FileSystem
{
public:
    MBootModuleFileSystem(const multiboot_info* mbootInfo);

protected:
    Stream* openStream(const char* path) override;

private:
    uintptr_t moduleStartAddr;
    size_t numModules;

    static constexpr size_t MAX_NUM_STREAMS = 16;
    MBootModuleStream streams[MAX_NUM_STREAMS];
};

#endif // MBOOT_MODULE_FILE_SYSTEM_H_
