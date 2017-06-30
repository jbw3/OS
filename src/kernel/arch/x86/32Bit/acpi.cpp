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

    /**
     * @brief Returns true if the given 4-character signature
     * matches the signature in the header
     */
    bool matchesSignature(char* signature)
    {
        for (int i = 0; i < 4; i++)
        {
            if (this->Signature[i] != signature[i])
            {
                return false;
            }
        }
        return true;    // first 4 chars matched
    }
} __attribute__((packed));

/**
 * @brief Length of ACPI System Description Table Header
 */
const uint32_t DESCR_HEADER_LENGTH = 36;

/**
 * ACPI RSDT Structure
 */
struct RootSystemDescriptionTable
{
    DESCRIPTION_HEADER Header;
    DESCRIPTION_HEADER* Entry;     // first pointer in array of header pointers

    DESCRIPTION_HEADER** getEntries()
    {
        return &Entry;
    }

    /**
     * @brief Returns the number of entries in the table
     */
    uint32_t count()
    {
        return (this->Header.Length - DESCR_HEADER_LENGTH) / 4;
    }
} __attribute__((packed));

/**
 * @brief ACPI MCFG config space base address struct
 * Taken from: http://wiki.osdev.org/PCI_Express
 */
struct BaseAddrAlloc    // cls: better name?
{
    uint64_t EnhancedConfigBaseAddress;     // Base address of enhanced configuration mechanism
    uint16_t PciSegmentGroupNumber;         // PCI Segment Group Number
    uint8_t StartPciBusNumber;              // Start PCI bus number decoded by this host bridge
    uint8_t EndPciBusNumber;                // End PCI bus number decoded by this host bridge
    uint32_t Reserved;
} __attribute__((packed));

/**
 * @brief ACPI MCFG Structure
 * Taken from: http://wiki.osdev.org/PCI_Express
 */
 struct MCFGTable
 {
     DESCRIPTION_HEADER Header;
     char Reserved[8];
     BaseAddrAlloc ConfigSpace;    // first entry in array of pointers

     BaseAddrAlloc* getConfigSpaceArray()
     {
         return &ConfigSpace;
     }

     /**
      * @brief Returns the number of ConfigSpace entries
      */
     uint32_t count()
     {
         return (this->Header.Length - DESCR_HEADER_LENGTH) / 16;
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

    uint32_t virtRsdtAddr = mem::toVirtualKernelAddr(RSDP->RsdtAddress);

    // RSDT Testing...
    acpi::RootSystemDescriptionTable* RSDT = (acpi::RootSystemDescriptionTable*)(virtRsdtAddr);

    screen << "VIRT ADDR: " << virtRsdtAddr << "\n";
    screen << "RSDT: ";
    RSDT->Header.printSignature();
    screen << "RSDT Length: " << os::Screen::dec << RSDT->Header.Length << "\n";
    screen << "Num entries: " << RSDT->count() << "\n\n";

    screen << os::Screen::hex;

    for (uint32_t i = 0; i < RSDT->count(); i++)
    {
        //screen << "RSDT address: " << RSDT << "\n";
        acpi::DESCRIPTION_HEADER** entries = RSDT->getEntries();
        acpi::DESCRIPTION_HEADER* entryHeader = entries[i];
        //screen << "entry[i] address: " << entryHeader << "\n";
        entryHeader = (acpi::DESCRIPTION_HEADER*)mem::toVirtualKernelAddr((uint32_t)entryHeader);
        entryHeader->printSignature();
        if (entryHeader->matchesSignature("MCFG"))
        {
            acpi::MCFGTable* mcfg = (acpi::MCFGTable*)(entryHeader);
            screen << "MCFG count: 0x" << mcfg->count() << "\n";

            acpi::BaseAddrAlloc* configSpaceArray = mcfg->getConfigSpaceArray();
            screen << "configSpaceArray: 0x" << configSpaceArray << "\n";
            screen << "configSpace[0] enhanced config base address: 0x" << configSpaceArray[0].EnhancedConfigBaseAddress << "\n";
        }
    }
}

}