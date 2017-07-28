#include "acpi.h"
#include "debug.h"
#include "pageframemgr.h"
#include "paging.h"
#include "string.h"
#include "system.h"
#include "vmem.h"

using namespace std;

namespace acpi {

void DESCRIPTION_HEADER::printSignature()
{
    for (int i = 0; i < 4; i++)
    {
        screen << this->Signature[i];
    }
    screen << "\n";
}

bool DESCRIPTION_HEADER::matchesSignature(const char* signature)
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

uint32_t RootSystemDescriptionTable::count()
{
    return (this->Header.Length - DESCR_HEADER_LENGTH) / 4;
}

uint32_t MCFGTable::count()
{
    return (this->Header.Length - DESCR_HEADER_LENGTH) / 16;
}

}

uint32_t getPciConfigSpace(uint32_t ecamBase, uint8_t bus, uint8_t device, uint8_t function=0)
{
    // cls todo: need to take the "starting bus #" into account here if the bus # for this
    // config area starts @ a nonzero # (e.g. bus 3 and up)
    return ecamBase + (bus << 20 | device << 15 | function << 12);
}

Acpi::Acpi()
    : _mcfgTable(nullptr)
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
            _mcfgTable = (acpi::MCFGTable*)(entryHeader);
            screen << "MCFG count: 0x" << _mcfgTable->count() << "\n";

            acpi::BaseAddrAlloc* configSpaceArray = _mcfgTable->getConfigSpaceArray();
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
            int NUM_PCI_CONFIG_SPACE_PAGES = 10;
            //int NUM_PCI_CONFIG_SPACE_PAGES = 65536;
            for (int i = 1; i < NUM_PCI_CONFIG_SPACE_PAGES; i++)    // skip first page...we already mapped it
            {
                // these should all be mapped in order, so we don't really care to save the
                // address here...
                uint32_t configPagePhysAddr = ecamPhysAddress + (i*4096);
                uint32_t configPageAddress = mem::autoMapKernelPageForAddress(configPagePhysAddr);
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
            //PANIC("here");
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
                    break;
                }
                break;  //tmp
            }
        }
    }
}

Acpi* Acpi::get()
{
    static Acpi _instance;
    return &_instance;
}

bool Acpi::mcfgTableExists()
{
    return _mcfgTable != nullptr;
}