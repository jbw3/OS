#pragma once

#include <stdint.h>

struct CAPRegister
{
    bool S64A       : 1;    // supports 64-bit addressing
    bool SNCQ       : 1;    // supports native command queuing
    bool SSNTF      : 1;    // supports SNotification Register (PxSNTF)
    bool SMPS       : 1;    // supports mechanical presence switch
    bool SSS        : 1;    // supports staggered spin-up
    bool SALP       : 1;    // supports aggressive link power management
    bool SAL        : 1;    // support activity LED
    bool SCLO       : 1;    // supports command list override
    int ISS         : 4;    // interface speed support
    bool reserved19 : 1;    // reserved flag bit 19
    bool SAM        : 1;    // supports AHCI mode only
    bool SPM        : 1;    // supports port multiplier
    bool FBSS       : 1;    // FIS-based switching supported
    bool PMD        : 1;    // PIO multiple IRQ block
    bool SSC        : 1;    // slumber state capable
    bool PSC        : 1;    // partial state capable
    int NCS         : 5;    // number of command slots, 0 => 1 slot, 1 => 2 slots, etc.
    bool CCCS       : 1;    // command completion coalescing support
    bool EMS        : 1;    // enclosure management support
    bool SXS        : 1;    // supports external SATA
    int NP          : 5;    // number of ports
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

struct AhciDeviceRegs
{
    GenericHostControlRegs genericHostControl;
    char reserved[52];
    char reservedForNVMHCI[64];
    char vendorSpecific[96];
    uint32_t* portRegs;     // todo: create array of PortRegs type
} __attribute__((packed));