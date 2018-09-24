#ifndef MBOOT_MODULE_FILE_SYSTEM_H_
#define MBOOT_MODULE_FILE_SYSTEM_H_

#include <stdint.h>
#include <stddef.h>
#include "filesystem.h"
#include "mbootmodulestream.h"

class MBootModuleFileSystem : public FileSystem
{
public:
    MBootModuleFileSystem(uintptr_t modulesStart, size_t numMods);

protected:
    Stream* openStream(const char* path) override;

private:
    uintptr_t moduleStartAddr;
    size_t numModules;

    static constexpr size_t MAX_NUM_STREAMS = 16;
    MBootModuleStream streams[MAX_NUM_STREAMS];
};

#endif // MBOOT_MODULE_FILE_SYSTEM_H_
