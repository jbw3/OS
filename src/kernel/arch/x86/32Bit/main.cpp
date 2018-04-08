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
#include "screen.h"
#include "serialportdriver.h"
#include "shell.h"
#include "streamtable.h"
#include "system.h"
#include "timer.h"
#include "userlogger.h"

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

    /// @todo temporary
    screen.setStream(&vgaDriver);

    // ensure we were booted by a Multiboot-compliant boot loader
    if (MULTIBOOT_MAGIC_NUM != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        const char* const ERROR_MSG_FORMAT = "Invalid Multiboot magic number: {x0>8}";
        klog.logError("Initialization", ERROR_MSG_FORMAT, MULTIBOOT_MAGIC_NUM);
        ulog.log(ERROR_MSG_FORMAT, MULTIBOOT_MAGIC_NUM);
        return;
    }

    // map multiboot module pages
    mapModules(mbootInfo);

    PageFrameMgr pageFrameMgr(mbootInfo);

    processMgr.setPageFrameMgr(&pageFrameMgr);
    processMgr.setMultibootInfo(mbootInfo);

    processMgr.mainloop();

    // Shell sh(mbootInfo);

    // while (true)
    // {
    //     os::Keyboard::processQueue();

    //     sh.update();

    //     // halt CPU until an interrupt occurs
    //     asm volatile ("hlt");
    // }
}
