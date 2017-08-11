#pragma once

#include "ahci.h"
#include "pci.h"

class AhciDriver
{
public:
    AhciDriver();

private:

    /**
     * @brief Returns true if the given PCI device supports
     * the AHCI specification.
     */
    static bool isAhciDevice(PciDevice* dev);

    /**
     * @brief Maps the given PCI device's AHCI register set
     * into virtual memory and returns a pointer to the
     * AhciDeviceRegs instance. The supplied PCI device must
     * be an AHCI device.
     * @returns a pointer to the AhciDeviceRegs instance if
     * successful. Returns nullptr if unsuccessful.
     */
    static ahci::AhciDeviceRegs* mapAhciDevice(PciDevice* dev);
};