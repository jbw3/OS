#include "ahcidriver.h"
#include "pageframemgr.h"
#include "pagetable.h"
#include "paging.h"
#include "pci.h"
#include "screen.h"
#include "system.h"
#include "vmem.h"

using namespace ahci;

AhciDriver::AhciDriver()
{
    // find any AHCI devices
    auto pci = Pci::get();

    for (int i = 0; i < pci->numDevices(); i++)
    {
        PciDevice* dev = &pci->devices()[i];
        if (isAhciDevice(dev))
        {
            screen << "found AHCI device\n";
            dev->printDeviceInfo(false);

            AhciDeviceRegs* ahciRegs = mapAhciDevice(dev);
            screen << "AHCI version: 0x" << ahciRegs->genericHostControl.VS << "\n";
            screen << "SAM: " << (int)ahciRegs->genericHostControl.CAP.SAM() << "\n";
            //screen << "NumPorts field: " << ahciRegs->genericHostControl.CAP.NP() << "\n";
            screen << "Ports Implemented: 0x" << ahciRegs->genericHostControl.PI << "\n";
            auto pi = ahciRegs->genericHostControl.PI;
            int numPorts = 0;
            for (int i = 0; i < (int)sizeof(pi)*8; i++)
            {
                numPorts += (int)((pi >> i) & 0x1);
            }
            screen << "# ports: " << numPorts << "\n";

            // TODO: MAP PORTS!!
            //mem::autoMapKernelPageForAddress()

            screen << "Port type: " << ahciRegs->portRegs[0].getSigString() << "\n";
            screen << os::Screen::dec;

            // max command slots I can allocate for each port
            screen << "# command slots: " << ahciRegs->genericHostControl.CAP.NumCommandSlots() << "\n";

            // allocate a page for the port command list and receive FIS
            uintptr_t pagePtrPhys = PageFrameMgr::get()->allocPageFrame();
            uint32_t pageAddrPhys = (uint32_t)pagePtrPhys;      // TODO: NEED TO KEEP PHYS ADDR FOR POINTING TO CMD LIST AND RECEIVE FIS
            mem::PageTable currentPT(mem::lastUsedKernelPDEIndex());
            if (currentPT.isFull())
            {
                PANIC("Page table full - not handling this properly in AHCI driver!");
            }
            uint32_t pageAddr = currentPT.mapNextAvailablePageToAddress(pageAddrPhys);
            uint32_t* pagePtr = (uint32_t*)pageAddr;

            screen << os::Screen::hex;
            screen << "sizeof(H2D): 0x" << sizeof(H2DFIS) << "\n";
            screen << "sizeof(CommandHeader): 0x" << sizeof(CommandHeader) << "\n";
            screen << "sizeof(PortSystemMemory): 0x" << sizeof(PortSystemMemory) << "\n";
            static_assert(sizeof(PortSystemMemory) <= 0x1000);
            // note: I can currently fit 2 PortSystemMemory structures in a single page,
            // or do 1 per page with some extra room to store other info at the bottom?

            // todo: set up command list
            ahciRegs->portRegs[0].PxCLB = pageAddrPhys;     // point to command list
            ahciRegs->portRegs[0].PxFB = pageAddrPhys + (sizeof(CommandHeader)*32);

            screen << "offsetof(): 0x" << sizeof(CommandHeader)*32 << "\n";

            // todo: set up receive FIS
            // todo: point PxCLB to command list (physical)
            // todo: point PxFB to receive FIS (physical)

            // set up AhciDevice instance for access later on...
            AhciDevice ahciDev;
            ahciDev._devRegs = ahciRegs;
            ahciDev._portMemoryPhysAddr[0] = pageAddrPhys;
            ahciDev._portMemory[0] = (PortSystemMemory*)pageAddr;

            // todo: place AhciDevice in array...?
        }
    }
}

bool AhciDriver::isAhciDevice(PciDevice* dev)
{
    return  dev->isHeaderType0() &&
            dev->header()->classCode == PCI_CLASS_MASS_STRG &&
            dev->header()->subclass == 0x06 &&   // SATA
            dev->header()->progIF == 0x01;       // AHCI
}

AhciDeviceRegs* AhciDriver::mapAhciDevice(PciDevice* dev)
{
    if (!isAhciDevice(dev))
    {
        return nullptr;
    }

    uint32_t ahciRegsPhysAddr = dev->headerType0()->BAR5;
    auto ahciRegsAddr = mem::autoMapKernelPageForAddress(ahciRegsPhysAddr);
    //screen << "map1: 0x" << ahciRegsAddr << " map2: 0x" << mem::autoMapKernelPageForAddress(ahciRegsPhysAddr + 0x100) << "\n";

    return (AhciDeviceRegs*)ahciRegsAddr;
}