#include "ahcidriver.h"
#include "pci.h"
#include "screen.h"
#include "vmem.h"

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

            AhciDeviceRegs* ahciDev = mapAhciDevice(dev);
            screen << "AHCI version: 0x" << ahciDev->genericHostControl.VS << "\n";
            screen << "SAM: " << (int)ahciDev->genericHostControl.CAP.SAM() << "\n";
            //screen << "NumPorts field: " << ahciDev->genericHostControl.CAP.NP() << "\n";
            screen << "Ports Implemented: 0x" << ahciDev->genericHostControl.PI << "\n";
            auto pi = ahciDev->genericHostControl.PI;
            int numPorts = 0;
            for (int i = 0; i < sizeof(pi)*8; i++)
            {
                numPorts += (int)((pi >> i) & 0x1);
            }
            screen << "# ports: " << numPorts << "\n";

            // TODO: MAP PORTS!!
            //mem::autoMapKernelPageForAddress()

            screen << "&portRegs: 0x" << (uint32_t)(&ahciDev->portRegs[0]) << "\n";
            screen << "sig: 0x";
            screen << ahciDev->portRegs[0].PxSIG << "\n";
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