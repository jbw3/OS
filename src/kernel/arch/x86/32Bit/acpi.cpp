#include <acpi.h>
#include <debug.h>
#include <paging.h>
#include <screen.h>
#include <system.h>

namespace os {

Acpi::Acpi()
{
    screen.write("ACPI Initializing...\n");

    // Read EBDA base address from BIOS Data Area
    uint16_t* ebdaBaseAddress = (uint16_t*)(0x40E + KERNEL_VIRTUAL_BASE);
    screen << os::Screen::hex << (*ebdaBaseAddress << 4) << os::Screen::dec << "\n";
    uint32_t* EBDAPtr = (uint32_t*)((*ebdaBaseAddress << 4) + KERNEL_VIRTUAL_BASE);

    while (*EBDAPtr != 0x52)
    {
        EBDAPtr++;
    }

    // found it
    for (int i = 0; i < 8; i++)
    {
        screen << os::Screen::hex << *EBDAPtr << os::Screen::dec;
        screen << "(" << (char)(*EBDAPtr++) << ") ";
    }
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