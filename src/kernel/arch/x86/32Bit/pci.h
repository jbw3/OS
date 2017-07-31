#pragma once

#include <stdint.h>

// mask for HeaderType field to determine if a device
// is multifunction
#define PCI_DEV_MULTIFUNCTION 0x80

// PCI device class codes
#define PCI_CLASS_NONE          0x00
#define PCI_CLASS_MASS_STRG     0x01
#define PCI_CLASS_NETWORK       0x02
#define PCI_CLASS_DISPLAY       0x03
#define PCI_CLASS_MULTIMEDIA    0x04
#define PCI_CLASS_MEMORY        0x05
#define PCI_CLASS_BRIDGE        0x06
#define PCI_CLASS_COMM          0x07
#define PCI_CLASS_BASE_SYSTEM   0x08
#define PCI_CLASS_INPUT         0x09
#define PCI_CLASS_DOCKING       0x0A
#define PCI_CLASS_PROCESSOR     0x0B
#define PCI_CLASS_SERIAL_BUS    0x0C
#define PCI_CLASS_WIRELESS      0x0D
#define PCI_CLASS_IO            0x0E
#define PCI_CLASS_SATELLITE     0x0F
#define PCI_CLASS_CRYPTO        0x10
#define PCI_CLASS_SIGNAL_PROC   0x11
#define PCI_CLASS_OTHER         0xFF

// cls: impose a limit on ourselves for now
// since we simply have an array of these
const int MAX_PCI_DEVICES = 64;

/**
 * @brief General PCI device header
 */
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
 * @brief PCI header for devices with header type 0 (general
 * PCI devices).
 */
struct PciHeader0
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
    uint32_t BAR0;
    uint32_t BAR1;
    uint32_t BAR2;
    uint32_t BAR3;
    uint32_t BAR4;
    uint32_t BAR5;
    uint32_t cardbusCISPointer;
    uint16_t subsystemVendorId;
    uint16_t subsystemId;
    uint32_t expansionROMBaseAddress;
    uint8_t capabilitiesPointer;
    char reserved[7];
    uint8_t interruptLine;
    uint8_t interruptPin;
    uint8_t minGrant;
    uint8_t maxLatency;
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
     * @brief Returns true if the device's header type is 0. This
     * was made a standalone function b/c it is easy to forget to
     * mask out the multifunction bit of the raw headerType field.
     */
    bool isHeaderType0();

    /**
     * @brief Returns a pointer to the config space
     * header for this device
     */
    PciDeviceHeader* header();

    /**
     * @brief Returns a pointer to the header type 0
     * config space for this device. This is only
     * valid if headerType() is 0.
     */
    PciHeader0* headerType0();

    void printDeviceInfo(bool brief=true);

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
    int _numDevices;
};