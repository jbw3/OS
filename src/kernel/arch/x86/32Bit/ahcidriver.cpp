#include "ahcidriver.h"
#include "pageframemgr.h"
#include "pagetable.h"
#include "paging.h"
#include "pci.h"
#include "screen.h"
#include "system.h"
#include "vmem.h"

#define PORT 0

using namespace ahci;

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

            AhciDeviceRegs* ahciRegs = mapAhciDevice(dev);

            // -------------------------------------------------------------
            // TODO:
            // Create an initHBA() function, move applicable code over, and
            // clean up everything. When cleaning up:
            // 1. Follow System SW Initialization Steps exactly to init HBA
            // 2. If that doesn't work, try an HBA reset
            // -------------------------------------------------------------

            ahciRegs->genericHostControl.GHC |= 1 << 31;    // set GHC.AE
            screen << "AHCI version: 0x" << ahciRegs->genericHostControl.VS << "\n";
            screen << "SAM: " << (int)ahciRegs->genericHostControl.CAP.SAM() << "\n";
            //screen << "NumPorts field: " << ahciRegs->genericHostControl.CAP.NP() << "\n";
            screen << "Ports Implemented: 0x" << ahciRegs->genericHostControl.PI << "\n";
            auto pi = ahciRegs->genericHostControl.PI;
            int numPorts = 0;
            for (int i = 0; i < (int)sizeof(pi)*8; i++)
            {
                numPorts += (int)((pi >> i) & 0x1);
            }
            screen << "# ports: " << numPorts << "\n";

            // TODO: MAP PORTS!!
            //mem::autoMapKernelPageForAddress()

            screen << "Port type: " << ahciRegs->portRegs[PORT].getSigString() << "\n";
            screen << os::Screen::dec;

            // max command slots I can allocate for each port
            screen << "# command slots: " << ahciRegs->genericHostControl.CAP.NumCommandSlots() << "\n";

            // allocate a page for the port command list and receive FIS
            uintptr_t pagePtrPhys = PageFrameMgr::get()->allocPageFrame();
            uint32_t pageAddrPhys = (uint32_t)pagePtrPhys;      // TODO: NEED TO KEEP PHYS ADDR FOR POINTING TO CMD LIST AND RECEIVE FIS
            mem::PageTable currentPT(mem::lastUsedKernelPDEIndex());
            if (currentPT.isFull())
            {
                PANIC("Page table full - not handling this properly in AHCI driver!");
            }
            uint32_t pageAddr = currentPT.mapNextAvailablePageToAddress(pageAddrPhys);
            uint32_t* pagePtr = (uint32_t*)pageAddr;

            screen << os::Screen::hex;
            screen << "sizeof(H2D): 0x" << sizeof(H2DFIS) << "\n";
            screen << "sizeof(CommandHeader): 0x" << sizeof(CommandHeader) << "\n";
            screen << "sizeof(PortSystemMemory): 0x" << sizeof(PortSystemMemory) << "\n";
            static_assert(sizeof(PortSystemMemory) <= 0x1000);
            // note: I can currently fit 2 PortSystemMemory structures in a single page,
            // or do 1 per page with some extra room to store other info at the bottom?

            screen << "PxCLB: " << ahciRegs->portRegs[PORT].PxCLB << "\n";
            screen << "PxFB: " << ahciRegs->portRegs[PORT].PxFB << "\n";

            ahciRegs->portRegs[PORT].PxCLB = pageAddrPhys;     // point to command list
            ahciRegs->portRegs[PORT].PxFB = pageAddrPhys + (sizeof(CommandHeader)*32);     // point to receive FIS

            screen << "PxCLB: " << ahciRegs->portRegs[PORT].PxCLB << "\n";
            screen << "PxFB: " << ahciRegs->portRegs[PORT].PxFB << "\n";

            screen << "Receive FIS Phys: 0x" << ahciRegs->portRegs[PORT].PxFB << "\n";

            screen << "sizeof(CommandHeader)*32: 0x" << sizeof(CommandHeader)*32 << "\n";

            // set up AhciDevice instance for access later on...
            AhciDevice ahciDev;
            ahciDev._devRegs = ahciRegs;
            ahciDev._portMemoryPhysAddr[PORT] = pageAddrPhys;
            ahciDev._portMemory[PORT] = (PortSystemMemory*)pageAddr;

            // test command
            CommandHeader* header = &ahciDev._portMemory[PORT]->CommandList[0];
            AhciPortRegs* regs = &ahciDev._devRegs->portRegs[PORT];
            screen << "Port 0 PxCI: 0x" << ahciDev._devRegs->portRegs[PORT].PxCI << "\n";
            screen << "PxCMD.ST: " << regs->PxCMD.ST() << "\n";
            screen << "PxCMD.CR: " << regs->PxCMD.CR() << "\n";
            screen << "PxCMD.FRE: " << regs->PxCMD.FRE() << "\n";
            screen << "BIOS owned semaphore: " << (ahciRegs->genericHostControl.BOHC & 0x1) << "\n";
            //header->

            //////////// TODO: Do all this the right way...
            // -------------------------------------------------------------
            // TODO: as you go, make sure all bits in regs/command headers/etc. are initialized to good defaults (0?)

            // TODO: verify that PxCMD.CR is 0

            // FROM SPEC:
            // Additionally, software shall not set PxCMD.ST to ‘1’ until a functional device is present on the port (as
            // determined by PxTFD.STS.BSY = ‘0’, PxTFD.STS.DRQ = ‘0’, and (PxSSTS.DET = 3h, or PxSSTS.IPM =
            // 2h or 6h or 8h))

            // TODO: set PxCMD.FRE to 1
            // TODO: set PxCMD.ST to 1
            // -------------------------------------------------------------

            // #################################
            // try identify device command
            // #################################

            // allocate command table page
            uint32_t commandTablePhys = (uint32_t)PageFrameMgr::get()->allocPageFrame();
            //mem::PageTable currentPT(mem::lastUsedKernelPDEIndex());
            if (currentPT.isFull())
            {
                PANIC("Page table full - not handling this properly in AHCI driver!");
            }

            // set up command table
            CommandTable* cmdTable = (CommandTable*)currentPT.mapNextAvailablePageToAddress(commandTablePhys);  // was pageAddrPhys...why???
            for (int i = 0; i < 16; i++)
            {
                cmdTable->_CommandPacket[i] = 0;
            }

            // allocate data buffer
            uint32_t dataBufferPhysAddr = PageFrameMgr::get()->allocPageFrame();
            if (currentPT.isFull())
            {
                PANIC("Page table full - not handling this properly in AHCI driver!");
            }

            char* dataBuffer = (char*)currentPT.mapNextAvailablePageToAddress(dataBufferPhysAddr);
            //screen << "DataBuffer phys: " << dataBufferPhysAddr << "\n";
            screen << "DataBuffer @0x" << (uint32_t)dataBuffer << "\n";

            dataBuffer[0] = 'c';
            dataBuffer[1] = 'a';
            dataBuffer[2] = 'l';
            dataBuffer[3] = 'e';
            dataBuffer[4] = 'b';

            cmdTable->getPRDTableArray()[0].DBA = dataBufferPhysAddr;    // phys address of buffer for identify data
            cmdTable->getPRDTableArray()[0].IOC(false);
            cmdTable->getPRDTableArray()[0].DBC(512-1);

            // create command FIS
            // todo: create zeroOut() function, then just set members that I'm actually using
            cmdTable->CommandFIS()->FISType = 0x27;     // identify device?
            cmdTable->CommandFIS()->Flags = 0xC;
            cmdTable->CommandFIS()->Command = 0xEC;
            cmdTable->CommandFIS()->Features = 0;
            cmdTable->CommandFIS()->LBA0_SectorNum = 0;
            cmdTable->CommandFIS()->LBA1_CylLow = 0;
            cmdTable->CommandFIS()->LBA2_CylHigh = 0;
            cmdTable->CommandFIS()->DevHead = 0xA0;
            cmdTable->CommandFIS()->LBA3_SectorNumExp = 0;
            cmdTable->CommandFIS()->LBA4_CylLowExp = 0;
            cmdTable->CommandFIS()->LBA5_CylHighExp = 0;
            cmdTable->CommandFIS()->FeaturesExp = 0;
            cmdTable->CommandFIS()->SectorCountLow = 0;
            cmdTable->CommandFIS()->SectorCountHigh = 0;
            cmdTable->CommandFIS()->Reserved = 0;
            cmdTable->CommandFIS()->Control = 0x08;
            cmdTable->CommandFIS()->Reserved2 = 0;

            // update command header
            header->PRDTL(1);   // one physical region descriptor table (PRDT length=1)
            header->PMP(0);     // no port multiplier

            // flags = 0
            header->C(0);
            header->B(0);
            header->R(0);
            header->P(0);
            header->W(0);
            header->A(0);

            // PRDBC initialized to 0 by software (me) here, but updated by hardware
            // as the transfer occurrs
            header->PRDBC(0);   // PRD byte count
            header->CFL(5);     // command FIS length of 5 DWORDS
            header->CTBA(commandTablePhys);     // set command table base address

            // check BSY and DRQ
            screen << "Ports Implemented: 0x" << ahciRegs->genericHostControl.PI << "\n";

            screen << "Port regs: " << (uint32_t)regs << "\n";
            screen << "BSY: " << (regs->PxTFD & (0x1 << 7)) << "\n";
            screen << "DRQ: " << (regs->PxTFD & (0x1 << 3)) << "\n";
            //screen << "PxCI: " << regs->PxCI << "\n";
            screen << "TFES: " << (regs->PxIS & (0x1 << 30)) << "\n";
            regs->PxCI |= 0x1;
            screen << "PxCI: " << regs->PxCI << "\n";
            screen << "PxCI: " << regs->PxCI << "\n";
            screen << "PxCI: " << regs->PxCI << "\n";
            screen << "TFES: " << (regs->PxIS & (0x1 << 30)) << "\n";

            // ##############################
            // TODO: check BIOS ownership, try device reset, check everything...
            // ##############################


            // pick a command header and go...
            //ahciDev._portMemory[0]->CommandList[0].PRDTL(5);
            // JUST PICK A SPOT AND HARDCODE A DEVICE THERE TO GET STARTED...
            // todo: place AhciDevice in array...?
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