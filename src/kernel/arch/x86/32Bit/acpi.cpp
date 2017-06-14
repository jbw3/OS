#include <acpi.h>
#include <debug.h>
#include <paging.h>
#include <screen.h>
#include <system.h>

namespace os {
    namespace acpi {

struct RootSystemDescriptionPointer
{
    char[8] Signature;
    uint8_t Checksum;
    char[6] OEMID;
    uint8_t Revision;   // current revision is 2
    uint32_t RsdtAddress;
    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    char[3] Reserved;
} __attribute__((packed));

    }   /// acpi

Acpi::Acpi()
{
    screen.write("ACPI Initializing...\n");

    // Read EBDA base address from BIOS Data Area
    uint16_t* ebdaBaseAddress = (uint16_t*)(0x40E + KERNEL_VIRTUAL_BASE);
    screen << os::Screen::hex << (*ebdaBaseAddress << 4) << os::Screen::dec << "\n";
    char* EBDAPtr = (char*)((*ebdaBaseAddress << 4) + KERNEL_VIRTUAL_BASE);

    bool found = false;
    while (!found)
    {
        EBDAPtr++;
        found = EBDAPtr[0] == 0x52 && EBDAPtr[1] == 0x53 && EBDAPtr[2] == 'D'
                && EBDAPtr[3] == ' ' && EBDAPtr[4] == 'P';
    }

    // found it
    screen << os::Screen::hex;
    screen << *EBDAPtr;
    screen << EBDAPtr[1];
    screen << EBDAPtr[2];
    screen << EBDAPtr[3];
    screen << EBDAPtr[4];
    screen << EBDAPtr[5];
    screen << EBDAPtr[6];
    // for (int i = 0; i < 8; i++)
    // {
    //     screen << os::Screen::hex << *EBDAPtr << os::Screen::dec;
    //     screen << "(" << (char)(*EBDAPtr++) << ") ";
    // }
    screen << "\n";

    int numPageDirs = 0;
    uint32_t* pageDir = getKernelPageDirStart();
    for (int i = 0; i < 1024; i++)
    {
        if (pageDir[i] & PAGE_DIR_PRESENT)
        {
            printPageDir(i, i);
            numPageDirs++;
        }
    }
    screen << "\nThere were " << numPageDirs << " page directories present." << "\n";
    screen << "CS Register: " << os::Screen::bin << getRegCS() << os::Screen::dec << "\n";
    screen << "CPL: " << currentPrivilegeLevel() << "\n";
}

}