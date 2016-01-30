#include "debug.h"
#include "paging.h"
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

    screen << os::Screen::setfill(' ');
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

void printPageDir(int startIdx, int endIdx)
{
    if (startIdx > endIdx)
    {
        screen << "Start index cannot be greater than end index\n";
        return;
    }

    screen << "Idx   Address   S  A  D  W  U  R  P\n"
              "----  --------  -  -  -  -  -  -  -\n";

    const uint32_t* pageDir = getPageDirStart();
    for (int i = startIdx; i <= endIdx; ++i)
    {
        uint32_t entry = pageDir[i];

        uint32_t address  = entry & PAGE_DIR_ADDRESS;
        bool pageSize     = entry & PAGE_DIR_PAGE_SIZE;
        bool accessed     = entry & PAGE_DIR_ACCESSED;
        bool cacheDisable = entry & PAGE_DIR_CACHE_DISABLE;
        bool writeThrough = entry & PAGE_DIR_WRITE_THROUGH;
        bool user         = entry & PAGE_DIR_USER;
        bool readWrite    = entry & PAGE_DIR_READ_WRITE;
        bool present      = entry & PAGE_DIR_PRESENT;

        screen << os::Screen::noboolalpha
               << os::Screen::setw(4) << i

               << "  "
               << os::Screen::hex
               << os::Screen::setfill('0')
               << os::Screen::setw(8)
               << address
               << os::Screen::setfill(' ')
               << os::Screen::dec

               << "  " << pageSize
               << "  " << accessed
               << "  " << cacheDisable
               << "  " << writeThrough
               << "  " << user
               << "  " << readWrite
               << "  " << present
               << '\n';
    }

}

void printPageTable(int pageDirIdx, int startIdx, int endIdx)
{
    if (pageDirIdx < 0 || pageDirIdx >= 1024)
    {
        screen << "Page directory index out of range\n";
        return;
    }

    if (startIdx > endIdx)
    {
        screen << "Start index cannot be greater than end index\n";
        return;
    }

    const uint32_t* pageDir = getPageDirStart();
    uint32_t pageDirEntry = pageDir[pageDirIdx];

    const uint32_t* pageTable = reinterpret_cast<const uint32_t*>(pageDirEntry & PAGE_DIR_ADDRESS);
    bool tablePresent = pageDirEntry & PAGE_DIR_PRESENT;
    if (!tablePresent)
    {
        screen << "No page table is mapped at index " << pageDirIdx << '\n';
        return;
    }

    screen << "Idx   Address   G  D  A  C  W  U  R  P\n"
              "----  --------  -  -  -  -  -  -  -  -\n";

    for (int i = startIdx; i <= endIdx; ++i)
    {
        uint32_t entry = pageTable[i];

        uint32_t address  = entry & PAGE_TABLE_ADDRESS;
        bool global       = entry & PAGE_TABLE_GLOBAL;
        bool dirty        = entry & PAGE_TABLE_DIRTY;
        bool accessed     = entry & PAGE_TABLE_ACCESSED;
        bool cacheDisable = entry & PAGE_TABLE_CACHE_DISABLE;
        bool writeThrough = entry & PAGE_TABLE_WRITE_THROUGH;
        bool user         = entry & PAGE_TABLE_USER;
        bool readWrite    = entry & PAGE_TABLE_READ_WRITE;
        bool present      = entry & PAGE_TABLE_PRESENT;

        screen << os::Screen::noboolalpha
               << os::Screen::setw(4)
               << i

               << "  "
               << os::Screen::hex
               << os::Screen::setfill('0')
               << os::Screen::setw(8)
               << address
               << os::Screen::setfill(' ')
               << os::Screen::dec

               << "  " << global
               << "  " << dirty
               << "  " << accessed
               << "  " << cacheDisable
               << "  " << writeThrough
               << "  " << user
               << "  " << readWrite
               << "  " << present
               << '\n';
    }
}
