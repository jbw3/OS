#include <stdint.h>

#include "multiboot.h"

#include "gdt.h"
#include "idt.h"
#include "irq.h"
#include "kernellogger.h"
#include "keyboard.h"
#include "pageframemgr.h"
#include "paging.h"
#include "processmgr.h"
#include "serialportdriver.h"
#include "streamtable.h"
#include "system.h"
#include "timer.h"
#include "unittests.h"
#include "userlogger.h"
#include "vgadriver.h"

/**
 * @brief 32-bit x86 kernel main
 */
extern "C"
void kernelMain(const uint32_t MULTIBOOT_MAGIC_NUM, const multiboot_info* mbootInfo)
{
    initGdt();
    initIdt();
    initIrq();
    configPaging();

    os::Timer::init(20);

    os::Keyboard::init();

    // enable interrupts
    asm volatile ("sti");

    // create stream drivers
    os::Keyboard keyboardDriver;
    VgaDriver vgaDriver;
    SerialPortDriver serial1(SerialPortDriver::COM1_PORT, 115'200);
    SerialPortDriver serial2(SerialPortDriver::COM2_PORT, 115'200);

    streamTable.addStream(&keyboardDriver);
    streamTable.addStream(&vgaDriver);
    streamTable.addStream(&serial1);
    streamTable.addStream(&serial2);

    ulog.addStream(&vgaDriver);
    ulog.addStream(&serial1);
    klog.setStream(&serial2);

    // ensure we were booted by a Multiboot-compliant boot loader
    if (MULTIBOOT_MAGIC_NUM != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        klog.logError("Initialization", "Invalid Multiboot magic number: {x0>8}", MULTIBOOT_MAGIC_NUM);
        ulog.log("Invalid Multiboot magic number: {x0>8}\n", MULTIBOOT_MAGIC_NUM);
        return;
    }

    // map multiboot module pages
    mapModules(mbootInfo);

    PageFrameMgr pageFrameMgr(mbootInfo);

    processMgr.setPageFrameMgr(&pageFrameMgr);
    processMgr.setMultibootInfo(mbootInfo);

    /// @todo Make a system call to run unit tests instead of doing it here.
    runUnitTests();

    processMgr.mainloop();
}
