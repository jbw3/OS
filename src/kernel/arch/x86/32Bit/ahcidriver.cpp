#include "ahcidriver.h"
#include "pageframemgr.h"
#include "pagetable.h"
#include "paging.h"
#include "pci.h"
#include "screen.h"
#include "system.h"
#include "timer.h"
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

            HBAMemoryRegs* hba = mapAhciDevice(dev);

            screen << os::Screen::hex;
            screen << "sizeof(GHC Regs): 0x" << sizeof(GenericHostControlRegs) << "\n";
            screen << "sizeof(HBAMemoryRegs): 0x:" << sizeof(HBAMemoryRegs) << "\n";
            screen << "sizeof(AhciPortRegs): 0x" << sizeof(AhciPortRegs) << "\n";
            return;

            initHBA(hba);

            screen << os::Screen::hex;
            for (int i = 0; i < 32; i++)
            {
                bool portImplemented = (hba->genericHostControl.PI >> i) & 0x1;
                if (portImplemented)
                {
                    screen << "PxSERR: " << hba->portRegs[i].PxSERR << "\n";
                }
            }

            // -------------------------------------------------------------
            // TODO:
            // Create an initHBA() function, move applicable code over, and
            // clean up everything. When cleaning up:
            // 1. Follow System SW Initialization Steps exactly to init HBA
            // 2. If that doesn't work, try an HBA reset
            // -------------------------------------------------------------

            // OLD INIT CODE (comment out as I go...)
            //hba->genericHostControl.GHC |= 1 << 31;    // set GHC.AE
            screen << "AHCI version: 0x" << hba->genericHostControl.VS << "\n";
            screen << "SAM: " << (int)hba->genericHostControl.CAP.SAM() << "\n";
            screen << "SPM: " << hba->genericHostControl.CAP.SPM() << "\n";
            screen << "PxIS: 0x" << hba->portRegs->PxIS.value << "\n";

            //screen << "NumPorts field: " << hba->genericHostControl.CAP.NP() << "\n";
            //screen << "Ports Implemented: 0x" << hba->genericHostControl.PI << "\n";
            auto pi = hba->genericHostControl.PI;
            int numPorts = 0;
            for (int i = 0; i < (int)sizeof(pi)*8; i++)
            {
                numPorts += (int)((pi >> i) & 0x1);
            }
            screen << "# ports: " << numPorts << "\n";

            // TODO: MAP PORTS!!
            //mem::autoMapKernelPageForAddress()

            screen << "Port type: " << hba->portRegs[PORT].getSigString() << "\n";
            screen << os::Screen::dec;

            // max command slots I can allocate for each port
            screen << "# command slots: " << hba->genericHostControl.CAP.NumCommandSlots() << "\n";

            // allocate a page for the port command list and receive FIS
            uintptr_t pagePtrPhys = PageFrameMgr::get()->allocPageFrame();
            uint32_t pageAddrPhys = (uint32_t)pagePtrPhys;      // TODO: NEED TO KEEP PHYS ADDR FOR POINTING TO CMD LIST AND RECEIVE FIS
            mem::PageTable currentPT(mem::lastUsedKernelPDEIndex());
            if (currentPT.isFull())
            {
                PANIC("Page table full - not handling this properly in AHCI driver!");
            }
            uint32_t pageAddr = currentPT.mapNextAvailablePageToAddress(pageAddrPhys);
            //uint32_t* pagePtr = (uint32_t*)pageAddr;

            screen << os::Screen::hex;
            screen << "sizeof(H2D): 0x" << sizeof(H2DFIS) << "\n";
            screen << "sizeof(CommandHeader): 0x" << sizeof(CommandHeader) << "\n";
            screen << "sizeof(PortSystemMemory): 0x" << sizeof(PortSystemMemory) << "\n";
            static_assert(sizeof(PortSystemMemory) <= 0x1000);
            // note: I can currently fit 2 PortSystemMemory structures in a single page,
            // or do 1 per page with some extra room to store other info at the bottom?

            screen << "PxCLB: " << hba->portRegs[PORT].PxCLB << "\n";
            screen << "PxFB: " << hba->portRegs[PORT].PxFB << "\n";

            // TMP: TRY TO USE LOCATIONS ALREADY MAPPED BY FIRMWARE
            uint32_t cmdListPtr = 0;
            uint32_t fbPtr = 0;

            if (!currentPT.isMapped(hba->portRegs[PORT].PxCLB, cmdListPtr))
            {
                cmdListPtr = currentPT.mapNextAvailablePageToAddress(hba->portRegs[PORT].PxCLB);
            }

            if (!currentPT.isMapped(hba->portRegs[PORT].PxFB, fbPtr))
            {
                fbPtr = currentPT.mapNextAvailablePageToAddress(hba->portRegs[PORT].PxFB);
            }

            screen << "CLB: 0x" << cmdListPtr << " FB: 0x" << fbPtr << "\n";

            // OLD STUFF - MAPPING IT TO MY NEW LOCATION...
            hba->portRegs[PORT].PxCLB = pageAddrPhys;     // point to command list
            hba->portRegs[PORT].PxFB = pageAddrPhys + (sizeof(CommandHeader)*32);     // point to receive FIS

            screen << "PxCLB: " << hba->portRegs[PORT].PxCLB << "\n";
            screen << "PxFB: " << hba->portRegs[PORT].PxFB << "\n";

            screen << "Receive FIS Phys: 0x" << hba->portRegs[PORT].PxFB << "\n";

            screen << "sizeof(CommandHeader)*32: 0x" << sizeof(CommandHeader)*32 << "\n";

            // set up AhciDevice instance for access later on...
            AhciDevice ahciDev;
            ahciDev._devRegs = hba;
            ahciDev._portMemoryPhysAddr[PORT] = pageAddrPhys;
            ahciDev._portMemory[PORT] = (PortSystemMemory*)pageAddr;

            // test command
            CommandHeader* header = &ahciDev._portMemory[PORT]->CommandList[0];
            //CommandHeader* commandList = (CommandHeader*)cmdListPtr;
            //CommandHeader* header = (CommandHeader*) ((uint32_t)(&commandList[0]) + 0x10);    // USE 2nd COMMAND HEADER
            screen << "sizeof(CommandHeader): 0x" << sizeof(CommandHeader) << "\n";
            // screen << (uint32_t)commandList << "\n";
            // screen << (uint32_t)header << "\n";


            screen << "CTBA: 0x" << header->CTBA() << "\n";

            //AhciPortRegs* regs = &ahciDev._devRegs->portRegs[PORT];
            AhciPortRegs* regs = &hba->portRegs[PORT];
            //screen << "Port 0 PxCI: 0x" << ahciDev._devRegs->portRegs[PORT].PxCI << "\n";
            screen << "PxCMD.ST: " << regs->PxCMD.ST() << "\n";
            screen << "PxCMD.CR: " << regs->PxCMD.CR() << "\n";
            screen << "PxCMD.FRE: " << regs->PxCMD.FRE() << "\n";
            screen << "PxIS.PCS: " << regs->PxIS.PCS() << "\n";
            //screen << "BIOS owned semaphore: " << (hba->genericHostControl.BOHC & 0x1) << "\n";
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
            screen << "DataBuffer phys: " << dataBufferPhysAddr << "\n";
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
            uint8_t* cmdFISPtr = (uint8_t*)cmdTable->CommandFIS();
            for (int i = 0; i < 0x50; i++)
            {
                cmdFISPtr[i] = 0;
            }

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
            screen << "Command Table Phys: 0x" << commandTablePhys << "\n";
            if (commandTablePhys % 128 != 0)
            {
                PANIC("command table not on 128-byte boundary!!");
            }

            // check BSY and DRQ
            //screen << "Ports Implemented: 0x" << hba->genericHostControl.PI << "\n";

            //screen << "Port regs: " << (uint32_t)regs << "\n";
            screen << "AE: " << (int)hba->genericHostControl.GHC.AE() << "\n";
            screen << "BSY: " << (regs->PxTFD & (0x1 << 7)) << "\n";
            screen << "DRQ: " << (regs->PxTFD & (0x1 << 3)) << "\n";
            screen << "PxCI: " << regs->PxCI << "\n";
            screen << "PxSACT: " << regs->PxSACT << "\n";
            screen << "PxCMD.ST: " << regs->PxCMD.ST() << "\n";
            //return;
            screen << "TFES: " << (regs->PxIS.value & (0x1 << 30)) << "\n";


            // --------------------------------------------------------
            // TODO: TURN ON DMA ENGINE (ST=1) BEFORE ISSUING COMMAND
            // --------------------------------------------------------


            //regs->PxCI |= 0x1;
            // per spec, only write 'new' bits to set to 1; the previous register contents should NOT be re-written in the
            // register write
            regs->PxCI = 0x1;
            screen << "PxCI: " << regs->PxCI << "\n";

            //return;     // TMP

            auto initialTicks = os::Timer::getTicks();
            while (os::Timer::getTicks() < (initialTicks+10))
            {
                // wait
            }
            screen << "500ms \n";

            if (!(regs->PxCI & 0x1))
            {
                // CI bit has been cleared by HBA
                screen << "Command processed!" << "\n";
                screen << "PRD byte count: " << header->PRDBC() << "\n";
                screen << "PxCI: " << regs->PxCI << "\n";
                screen << "TFES: " << (regs->PxIS.value & (0x1 << 30)) << "\n";
                // 16x32=512
                for (int i = 0; i < 32; i++)
                {
                    for (int j = 0; j < 16; j++)
                    {
                        screen << dataBuffer[(i*32)+j] << " ";
                    }
                    screen << "\n";
                    if (i == 10)
                        break;
                }
            }
            else
            {
                screen << "Command not processed within 500ms...\n";
            }

            // screen << "PxCI: " << regs->PxCI << "\n";
            // screen << "PxCI: " << regs->PxCI << "\n";
            // screen << "PxCI: " << regs->PxCI << "\n";


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

HBAMemoryRegs* AhciDriver::mapAhciDevice(PciDevice* dev)
{
    if (!isAhciDevice(dev))
    {
        return nullptr;
    }

    uint32_t ahciRegsPhysAddr = dev->headerType0()->BAR5;
    auto ahciRegsAddr = mem::autoMapKernelPageForAddress(ahciRegsPhysAddr);
    //screen << "map1: 0x" << ahciRegsAddr << " map2: 0x" << mem::autoMapKernelPageForAddress(ahciRegsPhysAddr + 0x100) << "\n";

    return (HBAMemoryRegs*)ahciRegsAddr;
}

void AhciDriver::initHBA(ahci::HBAMemoryRegs* hba)
{
    // 1. Indicate that system SW is AHCI aware by setting GHC.AE to 1
    hba->genericHostControl.GHC.AE(1);

    // 2. Determine which ports are implemented by reading the PI register
    screen << "Ports Implemented: 0x" << hba->genericHostControl.PI << "\n";

    // 3. Ensure controller is not in a running state by making sure all
    //    implemented ports are idle
    screen << os::Screen::dec;

    for (int i = 0; i < 32; i++)
    {
        bool portImplemented = (hba->genericHostControl.PI >> i) & 0x1;
        if (portImplemented)
        {
            screen << "Port " << i << ": ";
            screen << hba->portRegs[i].getSigString();

            if (hba->portRegs[i].isIdle())
            {
                screen << " (idle)\n";
            }
            else
            {
                screen << " (running)\n";

                // make idle
                screen << "Putting port " << i << " in an idle state...\n";
                screen << "CR: " << (int)hba->portRegs[i].PxCMD.CR() << " FRE: " << (int)hba->portRegs[i].PxCMD.FRE() << " FR: " << (int)hba->portRegs[i].PxCMD.FR() << "\n";

                hba->portRegs[i].PxCMD.ST(0);   // clear ST
                // wait 500ms, then verify CR goes to 0
                do
                {
                    auto initialTicks = os::Timer::getTicks();
                    while (os::Timer::getTicks() < (initialTicks+10))
                    {
                        // wait
                    }
                    screen << "500ms \n";
                }
                while (hba->portRegs[i].PxCMD.CR());

                screen << "CR == 0\n";

                if (hba->portRegs[i].PxCMD.FRE())
                {
                    hba->portRegs[i].PxCMD.FRE(0);  // clear FRE
                    // wait 500ms, then verify FR goes to 0
                    do
                    {
                        auto initialTicks = os::Timer::getTicks();
                        while (os::Timer::getTicks() < (initialTicks+10))
                        {
                            // wait
                        }
                        screen << "500ms \n";
                    }
                    while (hba->portRegs[i].PxCMD.FR());
                }

                screen << "FR == 0\n";
                screen << "CR: " << (int)hba->portRegs[i].PxCMD.CR() << " FRE: " << (int)hba->portRegs[i].PxCMD.FRE() << " FR: " << (int)hba->portRegs[i].PxCMD.FR() << "\n";

                //break;
            }
        }
    }
}