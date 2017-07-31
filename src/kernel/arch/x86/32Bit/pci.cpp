#include "acpi.h"
#include "pci.h"
#include "vmem.h"

/**
 * @brief Returns the physical address of the PCI config space corresponding to
 * the given PCI function.
 */
uint32_t getPciConfigSpace(uint32_t ecamBase, uint8_t bus, uint8_t device, uint8_t function=0)
{
    // cls todo: need to take the "starting bus #" into account here if the bus # for this
    // config area starts @ a nonzero # (e.g. bus 3 and up)
    return ecamBase + (bus << 20 | device << 15 | function << 12);
}

Pci::Pci()
    : _numDevices(0)
{
    if (!Acpi::get()->mcfgTableExists())
    {
        screen << "No MCFG table. Unable to probe PCIe bus\n";
    }

    auto mcfg = Acpi::get()->mcfgTable();

    screen << "MCFG count: 0x" << mcfg->count() << "\n";

    acpi::BaseAddrAlloc* configSpaceArray = mcfg->getConfigSpaceArray();

    for (int i = 0; i < mcfg->count(); i++)
    {
        screen << "segment group " << configSpaceArray[i].PciSegmentGroupNumber << "\n";
        screen << "ECAM config base address: 0x" << configSpaceArray[i].EnhancedConfigBaseAddress << "\n";
        screen << "start pci bus #: 0x" << configSpaceArray[i].StartPciBusNumber << "\n";
        screen << "end pci bus #: 0x" << configSpaceArray[i].EndPciBusNumber << "\n";

        uint32_t ecamPhysAddress = (uint32_t)(configSpaceArray[0].EnhancedConfigBaseAddress & 0x00000000FFFFFFFF);
        screen << "ecam base: 0x" << ecamPhysAddress << "\n";

        int bus = 0;
        //for (int bus = 0; bus < 256; bus++)
        {
            for (uint8_t device = 0; device < 32; device++)
            {
                for (uint8_t func = 0; func < 8; func++)
                {
                    uint32_t deviceConfigPhys = getPciConfigSpace(ecamPhysAddress, bus, device, func);
                    uint32_t deviceConfig = mem::autoMapKernelPageForAddress(deviceConfigPhys);

                    PciDevice dev(deviceConfig, bus, device, func);

                    if (dev.exists())
                    {
                        _devices[_numDevices++] = dev;  // save the device
                        if (!dev.multifunction())
                        {
                            break;  // skip the remaining functions
                        }
                    }
                    else if (func == 0)
                    {
                        // func 0 for this device DNE - skip the device
                        break;
                    }
                }
            }
        }
    }
}

Pci* Pci::get()
{
    static Pci _instance;
    return &_instance;
}

int Pci::numDevices()
{
    return _numDevices;
}

PciDevice* Pci::devices()
{
    return _devices;
}

PciDevice::PciDevice()
    : _configSpaceHeader(nullptr),
    _bus(0), _device(0), _function(0)
{
}

PciDevice::PciDevice(uint32_t configSpaceAddress, int bus, int device, int function)
    : _configSpaceHeader((PciDeviceHeader*)configSpaceAddress),
    _bus(bus), _device(device), _function(function)
{
}

bool PciDevice::exists()
{
    if (_configSpaceHeader != nullptr)
    {
        return header()->vendorId != 0xFFFF;
    }
    return false;
}

PciDeviceHeader* PciDevice::header()
{
    return _configSpaceHeader;
}

PciHeader0* PciDevice::headerType0()
{
    return (PciHeader0*)_configSpaceHeader;
}

bool PciDevice::multifunction()
{
    return header()->headerType & PCI_DEV_MULTIFUNCTION;
}

uint8_t PciDevice::headerType()
{
    return header()->headerType & (~PCI_DEV_MULTIFUNCTION);
}

bool PciDevice::isHeaderType0()
{
    return headerType() == 0;
}

void PciDevice::printDeviceInfo(bool brief)
{
    screen << _bus << "." << _device << "." << _function;
    screen << ": " << name() << "\n";
    screen << " id = " << header()->vendorId << ":" << header()->deviceId << " ";
    screen << "headerType = " << headerType() << "\n";
    screen << " MF = " << multifunction() << " ";
    screen << "class = " << header()->classCode << ":";
    screen << header()->subclass << " ";
    screen << "progIF = " << header()->progIF << "\n";

    if (!brief)
    {
        if (headerType() == 0)
        {
            screen << " BAR0: 0x" << headerType0()->BAR0 << "\n";
            screen << " BAR1: 0x" << headerType0()->BAR1 << "\n";
            screen << " BAR2: 0x" << headerType0()->BAR2 << "\n";
            screen << " BAR3: 0x" << headerType0()->BAR3 << "\n";
            screen << " BAR4: 0x" << headerType0()->BAR4 << "\n";
            screen << " BAR5: 0x" << headerType0()->BAR5 << "\n";
        }
    }
}

const char* PciDevice::name()
{
    switch (header()->classCode)
    {
        case PCI_CLASS_MASS_STRG:
            if (header()->subclass == 0x06)
            {
                if (header()->progIF == 0x00)
                {
                    return "SATA (vendor-specific)";
                }
                else if (header()->progIF == 0x01)
                {
                    return "SATA (AHCI)";
                }
            }
            break;
        case PCI_CLASS_NETWORK:
            if (header()->subclass == 0x00)
            {
                return "Ethernet Controller";
            }
            break;
        case PCI_CLASS_DISPLAY:
            if (header()->subclass == 0x00)
            {
                return "VGA";
            }
            break;
        case PCI_CLASS_BRIDGE:
            if (header()->subclass == 0x00)
            {
                return "Host Bridge";
            }
            else if (header()->subclass == 0x01)
            {
                return "ISA Bridge";
            }
            break;
        case PCI_CLASS_SERIAL_BUS:
            if (header()->subclass == 0x05)
            {
                return "SMBus";
            }
            break;
        default:
            return "UNKNOWN";
    }

    return "UNKNOWN";
}