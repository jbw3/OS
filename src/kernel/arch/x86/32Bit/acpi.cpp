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
    uint16_t PciSegmentGroupNumber;         // PCI Segment Group Number (allows access to > 256 bus segments)
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

uint32_t getPciConfigSpace(uint32_t ecamBase, uint8_t bus, uint8_t device, uint8_t function=0)
{
    // cls todo: need to take the "starting bus #" into account here if the bus # for this
    // config area starts @ a nonzero # (e.g. bus 3 and up)
    return ecamBase + (bus << 20 | device << 15 | function << 12);
}

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
            screen << "configSpace[0] segment group #: 0x" << configSpaceArray[0].PciSegmentGroupNumber << "\n";
            screen << "configSpace[0] start pci bus #: 0x" << configSpaceArray[0].StartPciBusNumber << "\n";
            screen << "configSpace[0] end pci bus #: 0x" << configSpaceArray[0].EndPciBusNumber << "\n";

            uint32_t ecamPhysAddress = (uint32_t)(configSpaceArray[0].EnhancedConfigBaseAddress & 0x00000000FFFFFFFF);
            uint32_t ecamBaseAddress = mem::toVirtualKernelAddr(ecamPhysAddress);
            screen << "ecam base: 0x" << ecamBaseAddress << "\n";

            // map entire ECAM memory area
            // 256 buses * 32 devices * 8 functions * 4KB config space = 256MB config space total
            // 256MB/4KB pages = 65536 pages
            int NUM_PCI_CONFIG_SPACE_PAGES = 65536;
            for (int i = 1; i < NUM_PCI_CONFIG_SPACE_PAGES; i++)    // skip first page...we already mapped it
            {
                // these should all be mapped in order, so we don't really care to save the
                // address here...
                uint32_t configPagePhysAddr = ecamPhysAddress + (i*4096);
                uint32_t configPageAddress = mem::autoMapKernelPageForAddress(configPagePhysAddr, _pageFrameMgr);
                if (configPagePhysAddr == getPciConfigSpace(ecamPhysAddress, 4, 0, 0))
                {
                    screen << "bus 4 0x" << configPagePhysAddr << " mapped to 0x" << configPageAddress << "\n";
                }
                //screen << "mapped 0x" << configPagePhysAddr << " to 0x" << configPageAddress << "\n";
            }

            // 0x114000 (next page table phys) is associated with 0xc0114000 using getPageTablePointer()
            // but the virt address 0xc0114000 is actually mapped to phys 0xbfe...something

            // 11.0000.0001 - 0x301
            // 11.0000.0010 - 0x302
            // c0000000
            //   114000
            // c0114000
            // 11.0100.0000  01.0001.0100
            // 340  114

            for (int bus = 0; bus < 256; bus++)
            {
                for (uint8_t device = 0; device < 32; device++)
                {
                    // tmp: hardcode function to 0
                    uint32_t deviceConfig = getPciConfigSpace(ecamBaseAddress, bus, device, 0);
                    //screen << "accessing 0x" << deviceConfig << "\n";

                    uint16_t* vendorId = (uint16_t*)(deviceConfig);
                    if (*vendorId != 0xFFFF)
                    {
                        //screen << "bus " << bus << ", device " << device << ": 0x" << deviceConfig;
                        //screen << " vendorID: 0x" << *vendorId << "\n";
                    }
                }
                //break;  //tmp
            }
        }
    }
}

}