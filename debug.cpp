#include "debug.h"
#include "screen.h"

struct multiboot_drives_entry
{
    uint32_t size;
    uint8_t drive_number;
    uint8_t drive_mode;
    uint16_t drive_cylinders;
    uint8_t drive_heads;
    uint8_t drive_sectors;
} __attribute__((packed));

void printMultibootInfo(const multiboot_info* mbootInfo)
{
    screen << "Multiboot info:\n";

    // print flags
    screen << os::Screen::bin
           << "Flags: " << mbootInfo->flags << '\n'
           << os::Screen::dec;

    uint32_t bit = 1;
    for (int i = 0; i < 32; ++i)
    {
        bool isSet = (mbootInfo->flags & bit);
        if (isSet)
        {
            screen << "Bit " << i << ": ";
            switch (bit)
            {
            case MULTIBOOT_INFO_MEMORY:
                screen << mbootInfo->mem_lower << " KB, " << mbootInfo->mem_upper << " KB\n";
                break;

            case MULTIBOOT_INFO_BOOTDEV:
                screen << os::Screen::hex
                       << mbootInfo->boot_device << '\n'
                       << os::Screen::dec;
                break;

            case MULTIBOOT_INFO_CMDLINE:
                screen << reinterpret_cast<const char*>(mbootInfo->cmdline) << '\n';
                break;

            case MULTIBOOT_INFO_MODS:
                screen << mbootInfo->mods_count << " boot module" << (mbootInfo->mods_count == 1 ? "" : "s") << " were loaded\n";
                break;

            case MULTIBOOT_INFO_AOUT_SYMS:
                screen << "AOUT\n";
                break;

            case MULTIBOOT_INFO_ELF_SHDR:
                screen << "ELF\n";
                break;

            case MULTIBOOT_INFO_DRIVE_INFO:
                screen << mbootInfo->drives_length << '\n';
                break;

            case MULTIBOOT_INFO_CONFIG_TABLE:
                screen << os::Screen::hex
                       << mbootInfo->config_table << '\n'
                       << os::Screen::dec;
                break;

            case MULTIBOOT_INFO_MEM_MAP:
                screen << mbootInfo->mmap_length << '\n';
                break;

            case MULTIBOOT_INFO_BOOT_LOADER_NAME:
                screen << reinterpret_cast<const char*>(mbootInfo->boot_loader_name) << '\n';
                break;

            default:
                screen << '\n';
                break;
            }
        }

        bit <<= 1;
    }
}

void printMemMap(uint32_t addr, uint32_t len)
{
    uint32_t offset = 0;
    while (offset < len)
    {
        const multiboot_mmap_entry* entry = reinterpret_cast<const multiboot_mmap_entry*>(addr + offset);

        uint64_t startAddr = entry->addr;
        uint64_t endAddr = entry->addr + entry->len - 1;

        screen << os::Screen::hex << os::Screen::setfill('0')
               << "0x" << os::Screen::setw(8) << startAddr
               << " - " << "0x" << os::Screen::setw(8) << endAddr
               << os::Screen::dec
               << " (" << (entry->len / 1024) << " KB)\n";

        offset += entry->size + sizeof(entry->size);
    }
}

void printDrives(uint32_t addr, uint32_t len)
{
    if (len == 0)
    {
        screen << "No drives\n";
    }
    else
    {
        uint32_t offset = 0;
        while (offset < len)
        {
            const multiboot_drives_entry* entry = reinterpret_cast<const multiboot_drives_entry*>(addr + offset);

            screen << "Drive " << entry->drive_number << ":\n"
                   << "Mode: " << entry->drive_mode << '\n'
                   << "Cylinders: " << entry->drive_cylinders << '\n'
                   << '\n';

            offset += entry->size;
        }
    }
}

void printMem(int ptr)
{
    printMem(reinterpret_cast<const uint32_t*>(ptr));
}

void printMem(const uint32_t* ptr)
{
    screen << os::Screen::hex << os::Screen::setfill('0')
           << os::Screen::setw(8) << ptr << ": "
           << os::Screen::setw(8) << *ptr << '\n'
           << os::Screen::dec;
}
