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
            auto maskedVal = (ahciDev->genericHostControl.CAP.value &
            (uint32_t)(0x40000));
            screen << "SAM check: " << maskedVal << "\n";

            uint32_t val = ahciDev->genericHostControl.CAP.value;
            for (int i = 31; i >= 0; i--)
            {
                //if (i == 18)
                {
                    screen << ((val >> i) & 0x1);
                }
            }
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

    return (AhciDeviceRegs*)ahciRegsAddr;
}