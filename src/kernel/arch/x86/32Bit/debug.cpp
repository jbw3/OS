#include "debug.h"
#include "memoryunits.h"
#include "paging.h"
#include "screen.h"
#include "system.h"

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
            {
                if (!isPagingEnabled())
                {
                    screen << reinterpret_cast<const char*>(mbootInfo->cmdline) << '\n';
                }
                break;
            }

            case MULTIBOOT_INFO_MODS:
                screen << mbootInfo->mods_count << " module" << (mbootInfo->mods_count == 1 ? "" : "s") << " loaded\n";
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
            {
                const char* bootLoaderName = reinterpret_cast<const char*>(mbootInfo->boot_loader_name + KERNEL_VIRTUAL_BASE);
                screen << bootLoaderName << '\n';
                break;
            }

            default:
                screen << '\n';
                break;
            }
        }

        bit <<= 1;
    }
}

void printMultibootModules(uint32_t addr, uint32_t len)
{
    addr += KERNEL_VIRTUAL_BASE;

    for (uint32_t i = 0; i < len; ++i)
    {
        const multiboot_mod_list* module = reinterpret_cast<const multiboot_mod_list*>(addr);

        const char* modName = reinterpret_cast<const char*>(module->cmdline + KERNEL_VIRTUAL_BASE);
        screen << modName << '\n';

        addr += sizeof(multiboot_mod_list);
    }
}

void printMultibootMemMap(uint32_t addr, uint32_t len)
{
    addr += KERNEL_VIRTUAL_BASE;

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
               << " (";

        uint64_t entryLen = entry->len;

        if (entryLen >= 1_MiB && entryLen % 1_MiB == 0)
        {
            screen << (entryLen / 1_MiB) << " MiB";
        }
        else if (entryLen >= 1_KiB && entryLen % 1_KiB == 0)
        {
            screen << (entryLen / 1_KiB) << " KiB";
        }
        else
        {
            screen << entryLen << " B";
        }

        screen << ")\n";

        offset += entry->size + sizeof(entry->size);
    }

    screen << os::Screen::setfill(' ');
}

void printMultibootDrives(uint32_t addr, uint32_t len)
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

    const uint32_t* pageDir = getKernelPageDirStart();
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

    const uint32_t* pageDir = getKernelPageDirStart();
    uint32_t pageDirEntry = pageDir[pageDirIdx];

    uint32_t physicalAddr = pageDirEntry & PAGE_DIR_ADDRESS;
    uint32_t virtualAddr = physicalAddr + KERNEL_VIRTUAL_BASE;
    const uint32_t* pageTable = reinterpret_cast<const uint32_t*>(virtualAddr);
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

#define MASTER_DRIVE 0xA0
#define SLAVE_DRIVE  0xB0

#define PORT_DATA         0x1F0
#define PORT_SECTOR_COUNT 0x1F2
#define PORT_LBA_LO       0x1F3
#define PORT_LBA_MID      0x1F4
#define PORT_LBA_HI       0x1F5
#define PORT_DRIVE_SELECT 0x1F6
#define PORT_COMMAND      0x1F7
#define PORT_STATUS       0x1F7

#define CMD_IDENTIFY      0xEC

#define STATUS_BSY        0x80
#define STATUS_DRQ        0x08
#define STATUS_ERR        0x01

static void printDrive(uint16_t drive)
{
    // select target drive
    outb(PORT_DRIVE_SELECT, drive);

    // set sector count, LBAlo, LBAmid, LBAhi IO ports TO 0
    outb(PORT_SECTOR_COUNT, 0);
    outb(PORT_LBA_LO,       0);
    outb(PORT_LBA_MID,      0);
    outb(PORT_LBA_HI,       0);

    // send identify command to command IO port
    outb(PORT_COMMAND, CMD_IDENTIFY);

    uint8_t status = inb(PORT_STATUS);

    // if status is 0, there is no drive
    if (status == 0x00)
    {
        screen << "No drive\n";
    }
    // if status is 0xFF, this is a "floating bus"
    else if (status == 0xFF)
    {
        screen << "Floating bus\n";
    }
    else
    {
        // wait for busy bit to clear
        while (status & STATUS_BSY)
        {
            status = inb(PORT_STATUS);
        }

        // wait for either the data request to complete or the error bit to be set
        while ( (status & STATUS_DRQ) == 0 && (status & STATUS_ERR) == 0)
        {
            status = inb(PORT_STATUS);
        }

        // read the data if the request was successful and there were no errors
        if ( (status & STATUS_DRQ) != 0 && (status & STATUS_ERR) == 0)
        {
            uint16_t data[256];
            uint16_t* dataPortPtr = reinterpret_cast<uint16_t*>(PORT_DATA);

            // read 256 16-bit values
            screen << os::Screen::hex << os::Screen::setfill('0');
            for (int i = 0; i < 256; ++i)
            {
                data[i] = *dataPortPtr;

                if (i % 26 == 0)
                {
                    screen << '\n';
                }
                screen << data[i] << ' ';
            }
            screen << os::Screen::dec << os::Screen::setfill(' ') << '\n';
        }
    }

    screen << "Status: "
           << os::Screen::hex
           << os::Screen::setw(2)
           << os::Screen::setfill('0')
           << status
           << os::Screen::setfill(' ')
           << os::Screen::dec
           << '\n';
}

void printDrives(bool printMaster /* = true */, bool printSlave /* = true */)
{
    if (printMaster)
    {
        screen << "Master drive:\n";
        printDrive(MASTER_DRIVE);
    }

    if (printSlave)
    {
        screen << "Slave drive:\n";
        printDrive(SLAVE_DRIVE);
    }
}
