#include <string.h>
#include "mbootmodulefilesystem.h"
#include "multiboot.h"
#include "streamtable.h"
#include "system.h"

MBootModuleFileSystem::MBootModuleFileSystem(const multiboot_info* mbootInfo) :
    moduleStartAddr(mbootInfo->mods_addr + KERNEL_VIRTUAL_BASE),
    numModules(mbootInfo->mods_count)
{
}

Stream* MBootModuleFileSystem::openStream(const char* path)
{
    // search for a module with a matching name
    const multiboot_mod_list* module = nullptr;
    uintptr_t addr = moduleStartAddr;
    for (size_t i = 0; i < numModules; ++i)
    {
        // get module info struct
        const multiboot_mod_list* modulePtr = reinterpret_cast<const multiboot_mod_list*>(addr);

        // check if name matches
        const char* modName = reinterpret_cast<const char*>(modulePtr->cmdline + KERNEL_VIRTUAL_BASE);
        if (strcmp(path, modName) == 0)
        {
            module = modulePtr;
            break;
        }

        addr += sizeof(multiboot_mod_list);
    }

    // find a stream
    MBootModuleStream* stream = nullptr;
    if (module != nullptr)
    {
        for (size_t i = 0; i < MAX_NUM_STREAMS && stream == nullptr; ++i)
        {
            if (!streams[i].isOpen())
            {
                stream = &streams[i];
            }
        }
    }

    // set the stream's module
    if (stream != nullptr)
    {
        stream->setModule(module);
    }

    return stream;
}
