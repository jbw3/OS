#include <acpi.h>
#include <debug.h>
#include <pageframemgr.h>
#include <paging.h>
#include <screen.h>
#include <system.h>

namespace os {
    namespace acpi {

struct RootSystemDescriptionPointer
{
    char Signature [8];
    uint8_t Checksum;
    char OEMID [6];
    uint8_t Revision;   // current revision is 2
    uint32_t RsdtAddress;
    uint32_t Length;
    uint64_t XsdtAddress;
    uint8_t ExtendedChecksum;
    char Reserved [3];
} __attribute__((packed));

    }   /// acpi

Acpi::Acpi(PageFrameMgr* pageFrameMgr)
    : _pageFrameMgr(pageFrameMgr)
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
    acpi::RootSystemDescriptionPointer* RSDP = (acpi::RootSystemDescriptionPointer*)(EBDAPtr);
    screen << RSDP->Signature[0];
    screen << RSDP->Signature[1];
    screen << RSDP->Signature[2];
    screen << RSDP->Signature[3];
    screen << RSDP->Signature[4];
    screen << RSDP->Signature[5];
    screen << RSDP->Signature[6];
    screen << RSDP->Signature[7] << "\n";

    screen << RSDP->OEMID[0];
    screen << RSDP->OEMID[1];
    screen << RSDP->OEMID[2];
    screen << RSDP->OEMID[3];
    screen << RSDP->OEMID[4];
    screen << RSDP->OEMID[5] << "\n";

    screen << "Revision: " << RSDP->Revision << "\n";
    screen << "RSDT Address: " << os::Screen::hex << RSDP->RsdtAddress << "\n";

    // need to map 0x7FE1000 page!!

    // VIRTUAL
    // 768 - 0x300 (dir index)
    // 1023 - 0x3FF (pagetab index)

    // VIRTUAL 0x3003FF/000 points to PHYSICAL 0x7FE1/000
    // TODO:
    // - page dir exists
    // - pagetab DNE -> ADD PAGE TABLE @ 1023
    // - mapping DNE -> MAP PAGE
    uint32_t* pageDir = getKernelPageDirStart();
    screen << os::Screen::dec;
    printPageDir(768, 768);
    printPageTable(768, 0x3FF, 0x3FF);
    uint32_t VIRTUAL_BASE = 0xC03F'F000;
    mapPage(pageDir, VIRTUAL_BASE, 0x07FE'1000);

    // mark physical memory as reserved
    _pageFrameMgr->reservePageFrame(0x7FE1000);

    char* rsdt = (char*)((RSDP->RsdtAddress - 0x07FE'1000) + VIRTUAL_BASE);
    screen << "test" << *rsdt << "\n";

    // for (int i = 0; i < 8; i++)
    // {111
    //     screen << os::Screen::hex << *EBDAPtr << os::Screen::dec;
    //     screen << "(" << (char)(*EBDAPtr++) << ") ";
    // }
    screen << "\n";

    int numPageDirs = 0;
    //uint32_t* pageDir = getKernelPageDirStart();
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