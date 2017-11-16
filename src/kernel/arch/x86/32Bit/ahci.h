#pragma once

#include <stdint.h>

namespace ahci {

// v1.0
struct CAPRegister
{
    uint32_t value;     // get entire register value

    inline bool S64A()  { return (value >> 31) & 0x1; } // supports 64-bit addressing
    inline bool SNCQ()  { return (value >> 30) & 0x1; } // supports native command queuing
    inline bool reserved29() { return (value >> 29) & 0x1; } // reserved
    inline bool SIS()   { return (value >> 28) & 0x1; } // supports interlock switch
    inline bool SSS()   { return (value >> 27) & 0x1; } // supports staggered spin-up
    inline bool SALP()  { return (value >> 26) & 0x1; } // supports aggressive link power management
    inline bool SAL()   { return (value >> 25) & 0x1; } // support activity LED
    inline bool SCLO()  { return (value >> 24) & 0x1; } // supports command list override
    inline int ISS()    { return (value >> 20) & 0xF; } // interface speed support
    inline bool SNZO() { return (value >> 19) & 0x1; }    // supports non-zero DMA offsets
    inline bool SAM()   { return (value >> 18) & 0x1; } // supports AHCI mode only
    inline bool SPM()   { return (value >> 17) & 0x1; } // supports port multiplier
    inline bool reserved16()  { return (value >> 16) & 0x1; } // reserved
    inline bool PMD()   { return (value >> 15) & 0x1; } // PIO multiple IRQ block
    inline bool SSC()   { return (value >> 14) & 0x1; } // slumber state capable
    inline bool PSC()   { return (value >> 13) & 0x1; } // partial state capable
    /**
     * @brief test
     */
    inline int NCS()    { return (value >> 8) & 0x1F; } // number of command slots, 0 => 1 slot, 1 => 2 slots, etc.
    inline int reserved7() { return (value >> 5) & 0x7; }
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

// v1.0
struct GHCRegister
{
    uint32_t value;     // get entire register value

    inline bool AE()    { return (value >> 31) & 0x1; }     // AHCI enable
    // bits 30:02 - reserved
    inline bool IE()    { return (value >> 1) & 0x1; }      // Interrupt enable
    inline bool HR()    { return (value >> 0) & 0x1; }      // HBA reset

    void AE(bool ae)    { value = ((ae & 0x1) << 31) | (value & 0x7FFF'FFFF); }
    void IE(bool ie)    { value = ((ie & 0x1) << 1)  | (value & 0xFFFF'FFFD); }
    void HR(bool hr)    { value = ((hr & 0x1) << 0)  | (value & 0xFFFF'FFFE); }

} __attribute__((packed));

// v1.0
struct GenericHostControlRegs
{
    CAPRegister CAP;       // capabilities
    GHCRegister GHC;       // global host control
    uint32_t IS;        // interrupt status
    uint32_t PI;        // ports implemented
    uint32_t VS;        // version
} __attribute__((packed));

// v1.0
struct PxCMDRegister
{
    uint32_t value;     // get entire register value

    // todo: fill in remaining bits...

    inline int CR()     { return (value >> 15) & 0x1; } // command list running
    inline int FR()     { return (value >> 14) & 0x1; } // FIS receive running
    inline int FRE()    { return (value >> 4) & 0x1; }  // FIS receive enable
    inline int ST()     { return (value >> 0) & 0x1; }  // start

    void FRE(bool fre) { value = ((fre & 0x1) << 4) | (value & 0xFFFF'FFEF); }
    void ST(bool st) { value = ((st & 0x1) << 0) | (value & 0xFFFF'FFFE); }

} __attribute__((packed));

// v1.0
struct PxISRegister
{
    uint32_t value;     // get entire register value

    inline int PCS()    { return (value >> 6) & 0x1; }  // port connect change status

} __attribute__((packed));

// v1.0
struct AhciPortRegs
{
    uint32_t PxCLB;     // command list base address
    uint32_t PxCLBU;    // command list base address (upper)
    uint32_t PxFB;      // FIS base address
    uint32_t PxFBU;     // FIS base address (upper)
    PxISRegister PxIS;      // interrupt status
    uint32_t PxIE;      // interrupt enable
    PxCMDRegister PxCMD;     // command and status
    uint32_t Reserved1C;    // reserved
    uint32_t PxTFD;     // task file data
    uint32_t PxSIG;     // signature
    uint32_t PxSSTS;    // SATA status
    uint32_t PxSCTL;    // SATA control
    uint32_t PxSERR;    // SATA error
    uint32_t PxSACT;    // SATA active
    uint32_t PxCI;      // command issue
    // reserved...
    char Reserved34 [0x34];
    char PxVS [0x10];     // vendor specific

    const char* getSigString()
    {
        switch (this->PxSIG)
        {
            case 0x0:
                return "Nothing attached";
            case 0x0101:
                return "SATA Device";
            case 0xEB140101:
                return "SATAPI Device";
            case 0xC33C0101:
                return "Enclosure Management Bridge";
            case 0x96690101:
                return "Port Multiplier";
            case 0xFFFFFFFF:
                return "Default register value";
            default:
                return "Unknown";
        }
    }

    /**
     * @brief Returns true if this port is idle
     */
    bool isIdle()
    {
        if (PxCMD.ST() || PxCMD.CR() || PxCMD.FRE() || PxCMD.FR())
        {
            return false;
        }
        else
        {
            return true;
        }
    }

} __attribute__((packed));

// v1.0
struct HBAMemoryRegs
{
    GenericHostControlRegs genericHostControl;
    char reserved[12];          // 00-1F (including GHC regs)
    char reserved2[0x80];       // 20-9F
    char vendorSpecific[0x60];  // A0-FF
    AhciPortRegs portRegs[32];
} __attribute__((packed));

/**
 * @brief AHCI Command Header
 */
struct CommandHeader
{
    // raw dwords
    uint32_t DW0;
    uint32_t DW1;
    uint32_t DW2;
    uint32_t DW3;
    uint32_t DW4;
    uint32_t DW5;
    uint32_t DW6;
    uint32_t DW7;

    int PRDTL() { return (DW0 >> 16) & 0xFFFF; }    // count of PRD table entries in command table
    int PMP()   { return (DW0 >> 12) & 0x000F; }
    int C()     { return (DW0 >> 10) & 0x0001; }
    int B()     { return (DW0 >>  9) & 0x0001; }
    int R()     { return (DW0 >>  8) & 0x0001; }
    int P()     { return (DW0 >>  7) & 0x0001; }
    int W()     { return (DW0 >>  6) & 0x0001; }
    int A()     { return (DW0 >>  5) & 0x0001; }
    int CFL()   { return (DW0 >>  0) & 0x001F; }

    void PRDTL(int value)   { DW0 = ((value & 0xFFFF)   << 16) | (DW0 & 0x0000'FFFF); }
    void PMP(int value)     { DW0 = ((value & 0xF)      << 12) | (DW0 & 0xFFFF'0FFF); }
    void C(int value)       { DW0 = ((value & 0x1)      << 10) | (DW0 & 0xFFFF'FBFF); }
    void B(int value)       { DW0 = ((value & 0x1)      << 9)  | (DW0 & 0xFFFF'FDFF); }
    void R(int value)       { DW0 = ((value & 0x1)      << 8)  | (DW0 & 0xFFFF'FEFF); }
    void P(int value)       { DW0 = ((value & 0x1)      << 7)  | (DW0 & 0xFFFF'FF7F); }
    void W(int value)       { DW0 = ((value & 0x1)      << 6)  | (DW0 & 0xFFFF'FFBF); }
    void A(int value)       { DW0 = ((value & 0x1)      << 5)  | (DW0 & 0xFFFF'FFDF); }
    void CFL(int value)     { DW0 = ((value << 0) & 0x1F) | (DW0 & 0xFFFF'FFE0); }

    uint32_t PRDBC()    { return DW1; }     // byte count
    void PRDBC(uint32_t value) { DW1 = value; }

    // Command Table Physical Address ------

    uint32_t CTBA()     { return DW2; }     // table base address (low)
    void CTBA(uint32_t value) { DW2 = value; }

    uint32_t CTBAU()    { return DW3; }     // table base address (high)
    void CTBAU(uint32_t value) { DW3 = value; }

} __attribute__((packed));

/**
 * @brief Contains port command list and receive FIS
 */
struct PortSystemMemory
{
    CommandHeader CommandList[32];
    uint8_t ReceiveFIS[256];    // Receive FIS is 256B (todo: make struct)
} __attribute__((packed));

/**
 * @brief Holds information about an AHCI device
 */
class AhciDevice
{
public:
// todo: make this private:
    HBAMemoryRegs* _devRegs;
    // indexed by port #
    PortSystemMemory* _portMemory[32];      // reduce?
    uint32_t _portMemoryPhysAddr[32];       // maintain physical addresses
};

/**
 * @brief Host2Device FIS
 */
struct H2DFIS
{
    uint8_t FISType;
    uint8_t Flags;      // bit 7 - command bit (new command), bit 3:0 - port multiplier
    uint8_t Command;
    uint8_t Features;
    uint8_t LBA0_SectorNum;
    uint8_t LBA1_CylLow;
    uint8_t LBA2_CylHigh;
    uint8_t DevHead;
    uint8_t LBA3_SectorNumExp;
    uint8_t LBA4_CylLowExp;
    uint8_t LBA5_CylHighExp;
    uint8_t FeaturesExp;
    uint8_t SectorCountLow;
    uint8_t SectorCountHigh;
    uint8_t Reserved;
    uint8_t Control;
    uint32_t Reserved2;
} __attribute__((packed));

/**
 * @brief Physical Region Descriptor
 */
struct PRD
{
    uint32_t DBA;       // data base address (low)
    uint32_t DBAU;      // data base address (upper)
    uint32_t Reserved;
    uint32_t Flags;     // 31 - interrupt on completion, 30:22 - reserved, 21:0 - data byte count

    bool IOC()  { return (Flags >> 31) & 0x1; }
    int DBC()   { return (Flags >>  0) & 0x3FFFFF; }

    void IOC(bool value) { Flags = ((value & 0x1) << 31) | (Flags & 0x7FFF'FFFF); }
    void DBC(int value)  { Flags = ((value & 0x3F'FFFF) << 0) | (Flags & 0xFFC0'0000); }

} __attribute__((packed));

struct CommandTable
{
    uint8_t _CommandFIS[0x40];      // space req'd for cmd FIS
    uint8_t _CommandPacket[0x10];   // space req'd for cmd packet...
    uint8_t _Reserved[0x30];
    PRD PRDTable;    // first entry in array of PRDs

    H2DFIS* CommandFIS() { return (H2DFIS*)(&_CommandFIS); }

    /**
     * @brief Returns a pointer to this Command Table's
     * PRD table (first entry)
     */
    PRD* getPRDTableArray() { return &PRDTable; }

} __attribute__((packed));

}   // ahci