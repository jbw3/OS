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

/**
 * @brief Sleep for ms milliseconds (needs to be 50ms multiple)
 *
 * @param ms
 */
void sleep(int ms)
{
    auto ticks = ms/50;     // 50ms increments
    auto initialTicks = os::Timer::getTicks();
    while (os::Timer::getTicks() < (initialTicks+ticks))
    {
        // wait
    }
}

static AhciDevice device;   // TMP - hardcoded until I get everything working

AhciDriver::AhciDriver()
{
    device._portMemory[0] = nullptr;

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
            device._devRegs = hba;  // TMP

            screen << os::Screen::hex;
            screen << "sizeof(GHC Regs): 0x" << sizeof(GenericHostControlRegs) << "\n";
            screen << "sizeof(HBAMemoryRegs): 0x:" << sizeof(HBAMemoryRegs) << "\n";
            screen << "sizeof(AhciPortRegs): 0x" << sizeof(AhciPortRegs) << "\n";
            screen << "sizeof(CommandHeader): 0x" << sizeof(CommandHeader) << "\n";

            screen << "Supports staggered spinup? " << hba->genericHostControl.CAP.SSS() << "\n";
            screen << "64-bit addressing? " << (int)hba->genericHostControl.CAP.S64A() << "\n";
            screen << "P0 idle? " << (int)hba->portRegs[PORT].isIdle() << "\n";
            screen << "PxCLB: " << hba->portRegs[PORT].PxCLB << "\n";
            screen << "PxFB: " << hba->portRegs[PORT].PxFB << "\n";
            screen << "HBA Reset: " << (int)hba->genericHostControl.GHC.HR() << "\n";
            screen << "HBA Reset=1...\n";

            // STATE: enter H:Init
            // ---------------------
            // - hbaIssueTag=0
            // - hbaDataTag=0
            // - hbaPMP=0
            // - hbaXferAtapi=0
            // - hbaPioXfer=???
            // - hbaPioESts=0
            // - hbaPioErr=0
            // - hbaPioIbit=0
            // - hbaDmaXferCnt=0
            // - hbaFatal=0
            // - hbaCmdToIssue=0
            // - hbaPrdIntr=0
            // - hbaUpdateSig=1
            // - hbaSActive=0
            // ---------------------
            // cls: my interpretation is that the hba performs a H:Init->H:NotRunning transition when GHC.HR goes to 0
            screen << "PxSIG: " << hba->portRegs[0].PxSIG << "\n";
            hba->genericHostControl.GHC.HR(1);
            screen << "PxSIG: " << hba->portRegs[0].PxSIG << "\n";

            sleep(500);
            screen << "HBA Reset: " << (int)hba->genericHostControl.GHC.HR() << "\n";
            screen << "PxSIG: " << hba->portRegs[0].PxSIG << "\n";

            // STATE: enter H:NotRunning
            // -------------------------
            // - hbaIssueTag=32
            // - z=CAP.NCS
            // -------------------------
            // Transitions I think are possible...
            // 7. PxCMD.ST = 1                  ....    enter H:Idle
            // 8. D2H Register FIS received     ....    enter NDR:Entry
            // 9. PxCMD.FRE 0 -> 1 AND
            //      register FIS is in receive FIFO AND
            //      PxSERR.DIAG.X = 0           ....    enter H:RegFisPostToMem

            initHBA(hba);
            screen << os::Screen::hex;
            screen << "PxSIG: " << hba->portRegs[0].PxSIG << "\n";

            // AFTER...
            screen << "RxFIS: " << (uint32_t)device._portMemory[0]->ReceiveFIS << "\n";
            for (int i = 0; i < 32; i++)
            {
                screen << device._portMemory[0]->ReceiveFIS[i] << " ";
            }
            screen << "\n";
            return;
            screen << "IS after clear: " << hba->genericHostControl.IS << "\n";
            screen << "CR: " << hba->portRegs[0].PxCMD.CR() << "\n";
            //return;

            /////////////////////////////////////////////////////////////
            // TODO:
            // - Check all steps in 10.3 (SW rules for DMA engines)
            // - Validate Identify Device Command format
            // - Check Receive FIS for anything interesting
            /////////////////////////////////////////////////////////////

            screen << os::Screen::hex;

            // OLD INIT CODE (comment out as I go...)
            // screen << "AHCI version: 0x" << hba->genericHostControl.VS << "\n";
            // screen << "SAM: " << (int)hba->genericHostControl.CAP.SAM() << "\n";
            // screen << "SPM: " << hba->genericHostControl.CAP.SPM() << "\n";
            // screen << "PxIS: 0x" << hba->portRegs->PxIS.value << "\n";

            // test command
            CommandHeader* header = &device._portMemory[PORT]->CommandList[0];
            screen << "CTBA: 0x" << header->CTBA() << "\n";

            AhciPortRegs* regs = &hba->portRegs[PORT];
            // screen << "PxCMD.ST: " << regs->PxCMD.ST() << "\n";
            // screen << "PxCMD.CR: " << regs->PxCMD.CR() << "\n";
            // screen << "PxCMD.FRE: " << regs->PxCMD.FRE() << "\n";
            // screen << "PxIS.PCS: " << regs->PxIS.PCS() << "\n";

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
            mem::PageTable currentPT(mem::lastUsedKernelPDEIndex());
            if (currentPT.isFull())
            {
                PANIC("Page table full - not handling this properly in AHCI driver!");
            }

            // set up command table
            CommandTable* cmdTable = (CommandTable*)currentPT.mapNextAvailablePageToAddress(commandTablePhys);  // was pageAddrPhys...why???
            uint8_t* cmdTableBytePtr = (uint8_t*)cmdTable;
            for (int i = 0; i < 0x60; i++)
            {
                cmdTableBytePtr[i] = 0;
            }
            for (int i = 0x80; i < 0x120; i++)
            {
                cmdTableBytePtr[i] = 0;
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

            cmdTable->getPRDTableArray()[0].DBAU = 0x0;
            cmdTable->getPRDTableArray()[0].DBA = dataBufferPhysAddr;    // phys address of buffer for identify data
            cmdTable->getPRDTableArray()[0].IOC(false);
            cmdTable->getPRDTableArray()[0].DBC(512-1);
            screen << "DataBufferPhys: 0x" << cmdTable->getPRDTableArray()->DBA << "\n";

            // create command FIS
            // todo: create zeroOut() function, then just set members that I'm actually using
            uint8_t* cmdFISPtr = (uint8_t*)cmdTable->CommandFIS();
            for (int i = 0; i < 0x60; i++)
            {
                cmdFISPtr[i] = 0;
            }

            // 0x27 - Register-H2D FIS Type
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
            // TODO: fix this some other way...
            if (header->CTBA() & 0x7F)
            {
                PANIC("CTBA not aligned!!");
            }
            screen << "CTBA: 0x" << commandTablePhys << "\n";

            screen << "Command Table Phys: 0x" << commandTablePhys << "\n";
            if (commandTablePhys % 128 != 0)
            {
                PANIC("command table not on 128-byte boundary!!");
            }

            // check BSY and DRQ
            //screen << "Ports Implemented: 0x" << hba->genericHostControl.PI << "\n";

            //screen << "Port regs: " << (uint32_t)regs << "\n";
            screen << "PxCI: " << regs->PxCI << " ";
            screen << "PxSACT: " << regs->PxSACT << " ";
            screen << "PxCMD.ST: " << regs->PxCMD.ST() << " ";
            screen << "TFES: " << (regs->PxIS.value & (0x1 << 30)) << "\n";

            // --------------------------------------------------------
            // TODO: TURN ON DMA ENGINE (ST=1) BEFORE ISSUING COMMAND
            // --------------------------------------------------------
            regs->PxCMD.ST(1);
            screen << "PxIS: " << regs->PxIS.value << "\n";
            sleep(500);    // wait after setting start?

            //regs->PxCI |= 0x1;
            // per spec, only write 'new' bits to set to 1; the previous register contents should NOT be re-written in the
            // register write
            regs->PxCI |= 0x1;
            screen << "PxCI: " << regs->PxCI << " ";
            screen << "PxCMD.ST: " << regs->PxCMD.ST() << "\n";

            sleep(500);
            screen << "500ms \n";
            screen << "PxSIG: " << regs->PxSIG << "\n";
            screen << "PxTFD.ERR: " << (int)(regs->PxTFD & 0x1) << "\n";

            if (!(regs->PxCI & 0x1))
            {
                // CI bit has been cleared by HBA
                screen << "Command processed!" << "\n";
                screen << "PRD byte count: " << header->PRDBC() << "\n";
                screen << "PxCI: " << regs->PxCI << "\n";
                // screen << "PxCMD.ST: " << regs->PxCMD.ST() << "\n";
                // screen << "PxSACT: " << regs->PxSACT << "\n";
                screen << "TFES: " << (regs->PxIS.value & (0x1 << 30)) << "\n";

                break;

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

                for (int i = 0; i < 32; i++)
                {
                    bool portImplemented = (hba->genericHostControl.PI >> i) & 0x1;
                    if (portImplemented)
                    {
                        // verify PxSERR.DIAG.X is cleared
                        screen << "PxSERR.DIAG.X: " << (hba->portRegs[i].PxSERR & (int)(0x1 << 26)) << " ";
                        screen << "PxSERR: " << hba->portRegs[i].PxSERR << " ";
                        // verify functional device is present on the port (BSY=0, DRQ=0, DET=3)
                        screen << "BSY: " << (hba->portRegs[i].PxTFD & (0x1 << 7)) << " DRQ: " << (hba->portRegs[i].PxTFD & (0x1 << 3)) << " ";
                        screen << "DET: " << (hba->portRegs[i].PxSSTS & 0x0F) << "\n";
                    }
                }
            }
            else
            {
                screen << "Command not processed within 500ms...\n";
            }

            screen << "PxTFD.ERR: " << (int)(regs->PxTFD & 0x1) << "\n";
            screen << "PxIE: " << regs->PxIE << " ";
            screen << "PxIS: " << regs->PxIS.value << " ";
            screen << "HBA IS: " << hba->genericHostControl.IS << " ";
            screen << "GHC.IE: " << hba->genericHostControl.GHC.IE() << "\n";
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
                continue;

                // make idle
                screen << "Putting port " << i << " in an idle state...\n";
                screen << "CR: " << (int)hba->portRegs[i].PxCMD.CR() << " FRE: " << (int)hba->portRegs[i].PxCMD.FRE() << " FR: " << (int)hba->portRegs[i].PxCMD.FR() << "\n";

                hba->portRegs[i].PxCMD.ST(0);   // clear ST
                // wait 500ms, then verify CR goes to 0
                do
                {
                    sleep(500);
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
                        sleep(500);
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

    // 4. Determine how many command slots are supported
    screen << "NCS: " << hba->genericHostControl.CAP.NCS() << "\n";     // is it NCS+1?

    // 5. For each implemented port, system SW shall set up PxCLB and PxFB by:
    //  - allocating memory
    //  - zero-ing out memory
    //  - setting port regs (PxCLB and PxFB) to the values of the physical addresses
    //  - set PxCMD.FRE = 1 after PxFB is set up
    for (int i = 0; i < 32; i++)
    {
        bool portImplemented = (hba->genericHostControl.PI >> i) & 0x1;
        if (portImplemented)
        {
            initAhciPort(&hba->portRegs[i]);
        }
    }

    // 6. For each implemented port, clear PxSERR register by writing 1's to each
    // implemented bit location
    for (int i = 0; i < 32; i++)
    {
        bool portImplemented = (hba->genericHostControl.PI >> i) & 0x1;
        if (portImplemented)
        {
            // screen << "PxSERR: " << hba->portRegs[i].PxSERR << "\n";
            hba->portRegs[i].PxSERR = 0xFFFF'FFFF;
            // screen << "PxSERR: " << hba->portRegs[i].PxSERR << "\n";
        }
    }

    // quick check to make sure this worked...
    // sleep(500);
    // for (int i = 0; i < 32; i++)
    // {
    //     bool portImplemented = (hba->genericHostControl.PI >> i) & 0x1;
    //     if (portImplemented)
    //     {
    //         screen << "PxSERR: " << hba->portRegs[i].PxSERR << "\n";
    //     }
    // }

    // 7. Configure interrupts (set GHC.IE and appropriate PxIE regs as desired)
    for (int i = 0; i < 32; i++)
    {
        bool portImplemented = (hba->genericHostControl.PI >> i) & 0x1;
        if (portImplemented)
        {
            hba->portRegs[i].PxIS.value = 0xFFFF'FFFF;
        }
    }

    for (int i = 0; i < 32; i++)
    {
        bool portImplemented = (hba->genericHostControl.PI >> i) & 0x1;
        if (portImplemented)
        {
            // screen << "PxIS after clear: " << hba->portRegs[i].PxIS.value << "\n";
        }
    }

    hba->genericHostControl.IS = 0xFFFF'FFFF;

    screen << "GHC.IE " << (int)hba->genericHostControl.GHC.IE() << "\n";
    hba->genericHostControl.GHC.IE(0);      // turn all interrupts off for now
    screen << "GHC.IE " << (int)hba->genericHostControl.GHC.IE() << "\n";

    // TODO: when setting interrupts, clear PxIS first, then clear IS.IPS BEFORE programming PxIE and GHC.IE

    // Final steps
    for (int i = 0; i < 32; i++)
    {
        bool portImplemented = (hba->genericHostControl.PI >> i) & 0x1;
        if (portImplemented)
        {
            // verify PxSERR.DIAG.X is cleared
            screen << "PxIS: " << hba->portRegs[i].PxIS.value << " ";
            screen << "PxSERR.DIAG.X: " << (hba->portRegs[i].PxSERR & (int)(0x1 << 26)) << " ";
            // verify functional device is present on the port (BSY=0, DRQ=0, DET=3)
            screen << "BSY: " << (hba->portRegs[i].PxTFD & (0x1 << 7)) << " DRQ: " << (hba->portRegs[i].PxTFD & (0x1 << 3)) << " ";
            screen << "DET: " << (hba->portRegs[i].PxSSTS & 0x0F) << "\n";
        }
    }
}

void AhciDriver::initAhciPort(ahci::AhciPortRegs* port)
{
    // allocate memory for port
    // allocate a page for the port command list and receive FIS
    uintptr_t pagePtrPhys = PageFrameMgr::get()->allocPageFrame();
    uint32_t pageAddrPhys = (uint32_t)pagePtrPhys;      // NEED TO KEEP PHYS ADDR FOR POINTING TO CMD LIST AND RECEIVE FIS

    mem::PageTable currentPT(mem::lastUsedKernelPDEIndex());
    if (currentPT.isFull())
    {
        PANIC("Page table full - not handling this properly in AHCI driver!");
    }
    uint32_t pageAddr = currentPT.mapNextAvailablePageToAddress(pageAddrPhys);

    // zero-out page memory (best practice move here...)
    uint32_t* pagePtr = (uint32_t*)pageAddr;    // zero out 4B at a time...

    for (int i = 0; i < (PAGE_SIZE/sizeof(uint32_t)); i++)
    {
        pagePtr[i] = 0;
    }

    screen << os::Screen::hex;
    screen << "PxCMD.FRE: " << port->PxCMD.FRE() << "\n";
    screen << "PxFB: " << port->PxFB << "\n";
    //PANIC("tmp");

    // set port regs to physical addresses...
    port->PxCLB = pageAddrPhys;                                 // point to phys address of command list (start of new page)
    port->PxFB = pageAddrPhys + (sizeof(CommandHeader)*32);     // point to receive FIS (directly after command list, on new page)
    screen << "PxFB: " << port->PxFB << "\n";

    // STATE NOTES
    // ------------------
    // It is possible that a D2H register FIS has already been received by the
    // HBA and is in the HBA's FIFO. If so, setting FRE to 1 will take us to
    // H:RegFisPostToMem (if PxSERR.DIAG.X = 0) and copy the register D2H FIS
    // into our PxFB buffer.

    // check before/after to see if this is the case...
    screen << "PxSERR.DIAG.X: " << (port->PxSERR & (int)(0x1 << 26)) << "\n";

    uint8_t* rxFISPtr = (uint8_t*)pagePtr;
    int startIdx = sizeof(CommandHeader)*32;

    // BEFORE...
    screen << os::Screen::hex;
    screen << "pagePtr: 0x" << (uint32_t)pagePtr << "\n";
    screen << "RxFIS: 0x" << (uint32_t)(&rxFISPtr[startIdx]) << "\n";
    for (int i = startIdx; i < startIdx+32; i++)
    {
        screen << rxFISPtr[i] << " ";
    }
    screen << "\n";

    // set PxCMD.FRE = 1 after setup is complete
    port->PxCMD.FRE(1);

    ///////////////////////////////////////////////////////////////////
    // D2H Register FIS is being received in RxFIS immediately after
    // setting FRE = 1 (RIGHT HERE!)
    //
    //  => We have transitioned to H:RegFisPostToMem as described above
    //
    // TODO: Parse out this register FIS and see if the contents are
    // interesting...
    ///////////////////////////////////////////////////////////////////

    //sleep(100);
    PANIC("TMP");

    // command table = sizeof(CommandHeader)*32
    // receive FIS = 256B
    auto cmdTableSize = sizeof(CommandHeader)*32;
    auto rxFISSize = 256;
    auto totalSize = cmdTableSize + rxFISSize;

    screen << os::Screen::dec;
    screen << "AHCI port memory utilizing " << totalSize << "B of " << PAGE_SIZE << "B available\n";
    // screen << "sizeof(PortMemory): " << sizeof(PortSystemMemory) << "\n";
    // screen << "Remaining space: " << PAGE_SIZE - totalSize << "B (" << (double)(PAGE_SIZE - totalSize)/(double)PAGE_SIZE << "%)\n";

    // screen << "PxCLB: " << port->PxCLB << "\n";
    // screen << "PxFB: " << port->PxFB << "\n";

    // TODO: once I get this working...
    // set up AhciDevice instance for access later on...
    // AhciDevice ahciDev;
    // ahciDev._devRegs = hba;
    // ahciDev._portMemoryPhysAddr[PORT] = pageAddrPhys;
    // ahciDev._portMemory[PORT] = (PortSystemMemory*)pageAddr;
    if (device._portMemory[0] == nullptr)
    {
        device._portMemoryPhysAddr[0] = pageAddrPhys;
        device._portMemory[0] = (PortSystemMemory*)pageAddr;
    }

    screen << os::Screen::hex;
    //screen << "sizeof(H2D): 0x" << sizeof(H2DFIS) << "\n";
    //screen << "sizeof(CommandHeader): 0x" << sizeof(CommandHeader) << "\n";
    // screen << "sizeof(PortSystemMemory): 0x" << sizeof(PortSystemMemory) << "\n";
    static_assert(sizeof(PortSystemMemory) <= 0x1000);
    // note: I can currently fit 2 PortSystemMemory structures in a single page,
    // or do 1 per page with some extra room to store other info at the bottom?
}