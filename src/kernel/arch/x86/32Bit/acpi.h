#pragma once

#include <stdint.h>
#include "screen.h"

// forward-declarations
class PageFrameMgr;

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
 * @brief Length of ACPI System Description Table Header
 */
const uint32_t DESCR_HEADER_LENGTH = 36;

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

    void printSignature();

    /**
     * @brief Returns true if the given 4-character signature
     * matches the signature in the header
     */
    bool matchesSignature(const char* signature);

} __attribute__((packed));

/**
 * ACPI RSDT Structure
 */
struct RootSystemDescriptionTable
{
    DESCRIPTION_HEADER Header;
    DESCRIPTION_HEADER* Entry;     // first pointer in array of header pointers

    DESCRIPTION_HEADER** getEntries() { return &Entry; }

    /**
     * @brief Returns the number of entries in the table
     */
    uint32_t count();

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

     BaseAddrAlloc* getConfigSpaceArray() { return &ConfigSpace; }

     /**
      * @brief Returns the number of ConfigSpace entries
      */
     uint32_t count();

 } __attribute__((packed));

}   // acpi

/**
 * Provides ACPI functionality
 */
class Acpi
{
public:

    /**
     * @brief Returns a handle to the ACPI singleton
     */
    static Acpi* get();

    /**
     * @brief Returns true if the MCFG table exists
     */
    bool mcfgTableExists();

    /**
     * @brief Returns a pointer to the MCFG table
     */
    acpi::MCFGTable* mcfgTable() { return _mcfgTable; }

private:
    Acpi();
    acpi::MCFGTable* _mcfgTable;
};