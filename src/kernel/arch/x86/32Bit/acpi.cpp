#include <acpi.h>
#include <debug.h>
#include <pageframemgr.h>
#include <paging.h>
#include <screen.h>
#include <string.h>
#include <system.h>
#include <vmem.h>

using namespace std;

namespace os {
    namespace acpi {

/**
 * ACPI RSDP Structure
 */
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

    void printSignature()
    {
        for (int i = 0; i < 8; i++)
        {
            screen << this->Signature[i];
        }
        screen << "\n";
    }

    void printOEMId()
    {
        for (int i = 0; i < 6; i++)
        {
            screen << this->OEMID[i];
        }
        screen << "\n";
    }
} __attribute__((packed));

/**
 * ACPI System Description Table Header
 */
struct DESCRIPTION_HEADER
{
    char Signature [4];
    uint32_t Length;
    uint8_t Revision;
    uint8_t Checksum;
    char OEMID [6];
    char OEMTableId [8];
    uint32_t OEMRevision;
    char CreatorId [4];
    uint32_t CreatorRevision;

    void printSignature()
    {
        for (int i = 0; i < 4; i++)
        {
            screen << this->Signature[i];
        }
        screen << "\n";
    }
} __attribute__((packed));

/**
 * ACPI RSDT Structure
 */
struct RootSystemDescriptionTable
{
    DESCRIPTION_HEADER Header;
    //DESCRIPTION_HEADER* Entry;
    uint32_t Entry;

    /**
     * @brief Returns the number of entries in the table
     */
    uint32_t count()
    {
        const uint32_t HEADER_LENGTH = 36;
        return (this->Header.Length - HEADER_LENGTH) / 4;
    }
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
    RSDP->printSignature();
    RSDP->printOEMId();

    screen << "Revision: " << RSDP->Revision << "\n";
    screen << "RSDT Address: " << os::Screen::hex << RSDP->RsdtAddress << "\n";

    uint32_t* pageDir = getKernelPageDirStart();
    screen << os::Screen::hex;

    // todo: verify this isn't mapped already?
    uint32_t virtRsdtAddr = os::autoMapKernelPageForAddress(RSDP->RsdtAddress, _pageFrameMgr);

    // todo: create RSDT struct and access that...
    acpi::RootSystemDescriptionTable* RSDT = (acpi::RootSystemDescriptionTable*)(virtRsdtAddr);

    screen << "VIRT ADDR: " << virtRsdtAddr << "\n";
    screen << "RSDT: ";
    RSDT->Header.printSignature();
    screen << "RSDT Length: " << os::Screen::dec << RSDT->Header.Length << "\n";
    screen << "Num entries: " << RSDT->count() << "\n\n";

    screen << os::Screen::hex;

    for (int i = 0; i < RSDT->count(); i++)
    //for (int i = 0; i < 2; i++)
    {
        uint32_t* entry = (uint32_t*)(&RSDT->Entry + i);

        screen << "entry address: 0x" << entry << "\n";
        uint32_t virtTableAddr = os::autoMapKernelPageForAddress(*entry, _pageFrameMgr);
        screen << "entry[" << i << "]: 0x" << *entry << "\n";
        screen << "entry[" << i << "]: 0x" << virtTableAddr << "\n";

        acpi::DESCRIPTION_HEADER* entryHdr = (acpi::DESCRIPTION_HEADER*)(virtTableAddr);
        entryHdr->printSignature();
        //RSDT->Entry[i].printSignature();
    }
}

}