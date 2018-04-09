#include "debug.h"
#include "memoryunits.h"
#include "paging.h"
#include "system.h"
#include "userlogger.h"

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
    ulog.log("Multiboot info:\n");

    // print flags
    ulog.log("Flags: {b}\n", mbootInfo->flags);

    uint32_t bit = 1;
    for (int i = 0; i < 32; ++i)
    {
        bool isSet = (mbootInfo->flags & bit);
        if (isSet)
        {
            ulog.log("Bit {}: ", i);
            switch (bit)
            {
            case MULTIBOOT_INFO_MEMORY:
                ulog.log("{} KB, {} KB\n", mbootInfo->mem_lower, mbootInfo->mem_upper);
                break;

            case MULTIBOOT_INFO_BOOTDEV:
                ulog.log("{x}\n", mbootInfo->boot_device);
                break;

            case MULTIBOOT_INFO_CMDLINE:
            {
                ulog.log("{}\n", reinterpret_cast<const char*>(mbootInfo->cmdline + KERNEL_VIRTUAL_BASE));
                break;
            }

            case MULTIBOOT_INFO_MODS:
                ulog.log("{} module{} loaded\n", mbootInfo->mods_count, (mbootInfo->mods_count == 1 ? "" : "s"));
                break;

            case MULTIBOOT_INFO_AOUT_SYMS:
                ulog.log("AOUT\n");
                break;

            case MULTIBOOT_INFO_ELF_SHDR:
                ulog.log("ELF\n");
                break;

            case MULTIBOOT_INFO_DRIVE_INFO:
                ulog.log("{}\n", mbootInfo->drives_length);
                break;

            case MULTIBOOT_INFO_CONFIG_TABLE:
                ulog.log("{x}\n", mbootInfo->config_table);
                break;

            case MULTIBOOT_INFO_MEM_MAP:
                ulog.log("{}\n", mbootInfo->mmap_length);
                break;

            case MULTIBOOT_INFO_BOOT_LOADER_NAME:
            {
                const char* bootLoaderName = reinterpret_cast<const char*>(mbootInfo->boot_loader_name + KERNEL_VIRTUAL_BASE);
                ulog.log("{}\n", bootLoaderName);
                break;
            }

            default:
                ulog.log("\n");
                break;
            }
        }

        bit <<= 1;
    }
}

void printMultibootModules(uint32_t addr, uint32_t len)
{
    addr += KERNEL_VIRTUAL_BASE;

    // print header
    ulog.log("Start     End       Command Line\n"
             "--------  --------  --------------------\n");

    for (uint32_t i = 0; i < len; ++i)
    {
        const multiboot_mod_list* module = reinterpret_cast<const multiboot_mod_list*>(addr);

        // start and end addresses
        ulog.log("{0>x8}  {0>x8}  ", module->mod_start, module->mod_end);

        // command line
        const char* modName = reinterpret_cast<const char*>(module->cmdline + KERNEL_VIRTUAL_BASE);
        ulog.log("{}\n", modName);

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

        ulog.log("{0>x8} - {0>x8} (", startAddr, endAddr);

        uint64_t entryLen = entry->len;

        if (entryLen >= 1_MiB && entryLen % 1_MiB == 0)
        {
            ulog.log("{} MiB", entryLen / 1_MiB);
        }
        else if (entryLen >= 1_KiB && entryLen % 1_KiB == 0)
        {
            ulog.log("{} KiB", entryLen / 1_KiB);
        }
        else
        {
            ulog.log("{} B", entryLen);
        }

        ulog.log(")\n");

        offset += entry->size + sizeof(entry->size);
    }
}

void printMultibootDrives(uint32_t addr, uint32_t len)
{
    if (len == 0)
    {
        ulog.log("No drives\n");
    }
    else
    {
        uint32_t offset = 0;
        while (offset < len)
        {
            const multiboot_drives_entry* entry = reinterpret_cast<const multiboot_drives_entry*>(addr + offset);

            ulog.log("Drive {}:\n"
                     "Mode: {}\n"
                     "Cylinders: {}\n"
                     "\n",
                     entry->drive_number, entry->drive_mode, entry->drive_cylinders);

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
    ulog.log("{0>x8}: {0>x8}\n", ptr, *ptr);
}

void printPageDir(const uint32_t* pageDir, int startIdx, int endIdx)
{
    if (startIdx > endIdx)
    {
        ulog.log("Start index cannot be greater than end index\n");
        return;
    }

    ulog.log("Idx   Address   S  A  D  W  U  R  P\n"
             "----  --------  -  -  -  -  -  -  -\n");

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

        ulog.log("{>4}  {0>x8}  {b}  {b}  {b}  {b}  {b}  {b}  {b}\n",
                 i, address, pageSize, accessed, cacheDisable, writeThrough, user, readWrite, present);
    }

}

void printPageTable(const uint32_t* pageTable, int startIdx, int endIdx)
{
    if (startIdx < 0 || startIdx >= 1024)
    {
        ulog.log("Start index is out of range\n");
        return;
    }

    if (endIdx < 0 || endIdx >= 1024)
    {
        ulog.log("End index is out of range\n");
        return;
    }

    if (startIdx > endIdx)
    {
        ulog.log("Start index cannot be greater than end index\n");
        return;
    }

    ulog.log("Idx   Address   G  D  A  C  W  U  R  P\n"
             "----  --------  -  -  -  -  -  -  -  -\n");

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

        ulog.log("{>4}  {0>x8}  {b}  {b}  {b}  {b}  {b}  {b}  {b}  {b}\n",
                 i, address, global, dirty, accessed, cacheDisable, writeThrough, user, readWrite, present);
    }
}

void printPageTable(const uint32_t* pageDir, int pageDirIdx, int startIdx, int endIdx)
{
    if (pageDirIdx < 0 || pageDirIdx >= 1024)
    {
        ulog.log("Page directory index out of range\n");
        return;
    }

    if (startIdx > endIdx)
    {
        ulog.log("Start index cannot be greater than end index\n");
        return;
    }

    uint32_t pageDirEntry = pageDir[pageDirIdx];

    uint32_t physicalAddr = pageDirEntry & PAGE_DIR_ADDRESS;
    uint32_t virtualAddr = physicalAddr + KERNEL_VIRTUAL_BASE;
    const uint32_t* pageTable = reinterpret_cast<const uint32_t*>(virtualAddr);
    bool tablePresent = pageDirEntry & PAGE_DIR_PRESENT;
    if (!tablePresent)
    {
        ulog.log("No page table is mapped at index {}\n", pageDirIdx);
        return;
    }

    printPageTable(pageTable, startIdx, endIdx);
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
        ulog.log("No drive\n");
    }
    // if status is 0xFF, this is a "floating bus"
    else if (status == 0xFF)
    {
        ulog.log("Floating bus\n");
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
            for (int i = 0; i < 256; ++i)
            {
                data[i] = *dataPortPtr;

                if (i % 26 == 0)
                {
                    ulog.log("\n");
                }
                ulog.log("{0>x} ", data[i]);
            }
        }
    }

    ulog.log("Status: {0>x2}\n", status);
}

void printDrives(bool printMaster /* = true */, bool printSlave /* = true */)
{
    if (printMaster)
    {
        ulog.log("Master drive:\n");
        printDrive(MASTER_DRIVE);
    }

    if (printSlave)
    {
        ulog.log("Slave drive:\n");
        printDrive(SLAVE_DRIVE);
    }
}
