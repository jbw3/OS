#include <string.h>
#include "mbootmodulestream.h"
#include "multiboot.h"

MBootModuleStream::MBootModuleStream()
{
    setModule(nullptr);
}

void MBootModuleStream::setModule(const multiboot_mod_list* modulePtr)
{
    module = modulePtr;
    position = (module == nullptr) ? 0 : module->mod_start;
}

ssize_t MBootModuleStream::read(uint8_t* buff, size_t nbyte)
{
    if (position + nbyte >= module->mod_end)
    {
        nbyte = module->mod_end - position;
    }

    memcpy(buff, reinterpret_cast<const void*>(position), nbyte);
    position += nbyte;

    return static_cast<ssize_t>(nbyte);
}

void MBootModuleStream::close()
{
    setModule(nullptr);
}

bool MBootModuleStream::isOpen() const
{
    return module != nullptr;
}
