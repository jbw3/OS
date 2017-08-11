#pragma once

#include <stdint.h>

namespace ahci {

struct CAPRegister
{
    uint32_t value;     // get entire register value

    inline bool S64A()  { return (value >> 31) & 0x1; } // supports 64-bit addressing
    inline bool SNCQ()  { return (value >> 30) & 0x1; } // supports native command queuing
    inline bool SSNTF() { return (value >> 29) & 0x1; } // supports SNotification Register (PxSNTF)
    inline bool SMPS()  { return (value >> 28) & 0x1; } // supports mechanical presence switch
    inline bool SSS()   { return (value >> 27) & 0x1; } // supports staggered spin-up
    inline bool SALP()  { return (value >> 26) & 0x1; } // supports aggressive link power management
    inline bool SAL()   { return (value >> 25) & 0x1; } // support activity LED
    inline bool SCLO()  { return (value >> 24) & 0x1; } // supports command list override
    inline int ISS()    { return (value >> 20) & 0xF; } // interface speed support
    inline bool reserved19() { return (value >> 19) & 0x1; }    // reserved flag bit 19
    inline bool SAM()   { return (value >> 18) & 0x1; } // supports AHCI mode only
    inline bool SPM()   { return (value >> 17) & 0x1; } // supports port multiplier
    inline bool FBSS()  { return (value >> 16) & 0x1; } // FIS-based switching supported
    inline bool PMD()   { return (value >> 15) & 0x1; } // PIO multiple IRQ block
    inline bool SSC()   { return (value >> 14) & 0x1; } // slumber state capable
    inline bool PSC()   { return (value >> 13) & 0x1; } // partial state capable
    /**
     * @brief test
     */
    inline int NCS()    { return (value >> 8) & 0x1F; } // number of command slots, 0 => 1 slot, 1 => 2 slots, etc.
    inline bool CCCS()  { return (value >> 7) & 0x1; } // command completion coalescing support
    inline bool EMS()   { return (value >> 6) & 0x1; } // enclosure management support
    inline bool SXS()   { return (value >> 5) & 0x1; } // supports external SATA
    inline int NP()     { return (value >> 0) & 0x1F; } // number of ports, zero-based

    /**
     * @brief Returns the maximum number of ports available.
     * Check the PortsImplemented register for actual # ports
     */
    int NumPorts() { return NP() + 1; }

    /**
     * @brief Returns the number of command slots.
     */
    int NumCommandSlots() { return NCS() + 1; }

} __attribute__((packed));

struct GHCRegister
{
    bool EA : 1;        // AHCI enable

} __attribute__((packed));

struct GenericHostControlRegs
{
    CAPRegister CAP;       // capabilities
    uint32_t GHC;       // global host control
    uint32_t IS;        // interrupt status
    uint32_t PI;        // ports implemented
    uint32_t VS;        // version
    uint32_t CCC_CTL;   // command completion coalescing control
    uint32_t CCC_PORTS; // command completion coalescing ports
    uint32_t EM_LOC;    // enclosure management location
    uint32_t EM_CTL;    // enclosure management control
    uint32_t CAP2;      // extended capabilities
    uint32_t BOHC;      // BIOS/OS handoff control and status
} __attribute__((packed));

struct AhciPortRegs
{
    uint32_t PxCLB;     // command list base address
    uint32_t PxCLBU;    // command list base address (upper)
    uint32_t PxFB;      // FIS base address
    uint32_t PxFBU;     // FIS base address (upper)
    uint32_t PxIS;      // interrupt status
    uint32_t PxIE;      // interrupt enable
    uint32_t PxCMD;     // command and status
    uint32_t Reserved1C;    // reserved
    uint32_t PxTFD;     // task file data
    uint32_t PxSIG;     // signature
    uint32_t PxSSTS;    // SATA status
    uint32_t PxSCTL;    // SATA control
    uint32_t PxSERR;    // SATA error
    uint32_t PxSACT;    // SATA active
    uint32_t PxCI;      // command issue
    uint32_t PxSNTF;    // SATA notification
    uint32_t PxFBS;     // FIS-based switch controlling
    uint32_t PxDEVSLP;  // device sleep
    char Reserved48 [40];
    char PxVS [16];     // vendor specific
} __attribute__((packed));

struct AhciDeviceRegs
{
    GenericHostControlRegs genericHostControl;
    char reserved[52];
    char reservedForNVMHCI[64];
    char vendorSpecific[96];
    AhciPortRegs portRegs[32];
} __attribute__((packed));

/**
 * @brief AHCI Command Header
 */
struct CommandHeader
{
    uint32_t DW0;
    uint32_t DW1;
    uint32_t DW2;
    uint32_t DW3;
    uint32_t DW4;
    uint32_t DW5;
    uint32_t DW6;
    uint32_t DW7;

    // todo: methods to extract fields
} __attribute__((packed));

}   // ahci