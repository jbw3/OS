#include "ahcidriver.h"
#include "pageframemgr.h"
#include "pagetable.h"
#include "paging.h"
#include "pci.h"
#include "screen.h"
#include "system.h"
#include "vmem.h"

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

            screen << "Port type: " << ahciRegs->portRegs[0].getSigString() << "\n";
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


            ahciRegs->portRegs[0].PxCLB = pageAddrPhys;     // point to command list
            ahciRegs->portRegs[0].PxFB = pageAddrPhys + (sizeof(CommandHeader)*32);     // point to receive FIS

            screen << "sizeof(CommandHeader)*32: 0x" << sizeof(CommandHeader)*32 << "\n";

            // set up AhciDevice instance for access later on...
            AhciDevice ahciDev;
            ahciDev._devRegs = ahciRegs;
            ahciDev._portMemoryPhysAddr[0] = pageAddrPhys;
            ahciDev._portMemory[0] = (PortSystemMemory*)pageAddr;

            // test command
            CommandHeader* header = &ahciDev._portMemory[0]->CommandList[0];
            AhciPortRegs* regs = &ahciDev._devRegs->portRegs[0];
            screen << "Port 0 PxCI: 0x" << ahciDev._devRegs->portRegs[0].PxCI << "\n";
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
            CommandTable* cmdTable = (CommandTable*)currentPT.mapNextAvailablePageToAddress(pageAddrPhys);
            for (int i = 0; i < 16; i++)
            {
                cmdTable->_CommandPacket[i] = 0;
            }

            uint8_t dataBuffer[512];
            uint32_t dataBufferPhysAddr = 0;    // TODO: getPhysAddr(dataBuffer)

            cmdTable->getPRDTableArray()[0].DBA = dataBufferPhysAddr;    // phys address of buffer for identify data
            //cmdTable->getPRDTableArray()[0].Flags


            // --------------------------------
            // PICK UP HERE ^^^
            // --------------------------------



            // create command FIS
            cmdTable->CommandFIS()->FISType = 0x27;     // identify device?
            // TODO: finish...

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