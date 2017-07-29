#pragma once

// cls: impose a limit on ourselves for now
// since we simply have an array of these
const int MAX_PCI_DEVICES = 64;

struct PciDeviceHeader
{
    uint16_t vendorId;
    uint16_t deviceId;
    uint16_t command;
    uint16_t status;
    uint8_t revisionId;
    uint8_t progIF;
    uint8_t subclass;
    uint8_t classCode;
    uint8_t cacheLineSize;
    uint8_t latencyTimer;
    uint8_t headerType;
    uint8_t BIST;
} __attribute__((packed));

/**
 * @brief Represents a PCI device
 */
struct PciDevice
{
    /**
     * @brief Default constructor.
     */
    PciDevice();

    /**
     * @brief Creates a new PCI device instance using the
     * pointer to the device's PCI configuration space.
     */
    PciDevice(uint32_t configSpaceAddress, int bus, int device, int function);

    /**
     * @brief Returns true if the device exists on
     * the PCI bus
     */
    bool exists();

    /**
     * @brief Returns true if the device is multi-function
     */
    bool multifunction();

    /**
     * @brief Returns the header type without the multifunction
     * bit.
     */
    uint8_t headerType();

    /**
     * @brief Returns a pointer to the config space
     * header for this device
     */
    PciDeviceHeader* header();

    void printDeviceInfo();

    /**
     * @brief Returns a human-readable name for the device
     */
    const char* name();

private:
    PciDeviceHeader* _configSpaceHeader;
    int _bus, _device, _function;   // address
};

class Pci
{
public:
    /**
     * @brief Returns a pointer to the Pci singleton
     */
    static Pci* get();

    /**
     * @brief Returns the total number of PCI devices
     * in the system
     */
    int numDevices();

    /**
     * @brief Returns a pointer to the array of PCI devices.
     * Use numDevices() to get the size of this array.
     */
    PciDevice* devices();

private:
    Pci();
    PciDevice _devices [MAX_PCI_DEVICES];
};