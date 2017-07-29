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

int MCFGTable::count()
{
    return (this->Header.Length - DESCR_HEADER_LENGTH) / 16;
}

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