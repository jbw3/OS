#include <acpi.h>
#include <debug.h>
#include <pageframemgr.h>
#include <paging.h>
#include <screen.h>
#include <system.h>
#include <vmem.h>

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
    screen  << "EBDA Base: " << os::Screen::hex
            << (*ebdaBaseAddress << 4) << os::Screen::dec << "\n";
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

    uint32_t* pageDir = getKernelPageDirStart();
    screen << os::Screen::hex;

    uint32_t virtRsdtAddr = os::autoMapKernelPageForAddress(RSDP->RsdtAddress, _pageFrameMgr);

    // todo: create RSDT struct and access that...

    screen << "VIRT ADDR: " << virtRsdtAddr << "\n";
    screen << "RSDT: " << *(char*)(virtRsdtAddr) << "\n";
}

}