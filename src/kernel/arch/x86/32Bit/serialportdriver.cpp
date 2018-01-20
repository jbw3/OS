#include "screen.h"
#include "serialportdriver.h"
#include "system.h"

SerialPortDriver::SerialPortDriver(uint16_t portAddr)
{
    screen << "----- start serial init -----\n" << os::Screen::hex;

    port = portAddr;

    outb(port + LCR, 0);    // disable DLAB
    outb(port + IER, 0);    // disable interrupts
    outb(port + LCR, DLAB); // enable DLAB
    outb(port + DLL, 1);    // set baud divisor to 1 (115,200) (low byte)
    outb(port + DLH, 0);    // (high byte)
    outb(port + LCR, 0x03); // 8 bits, no parity, 1 stop bit
    outb(port + FCR, 0x87); // enable FIFOs, clear them, 8-byte trigger threshold
    outb(port + MCR, 0x0b); // enable IRQs, set RTS/DTR

    screen << "----- end serial init -----\n" << os::Screen::dec;

    for (int i = 0; i < 10; ++i)
    {
        while ( (inb(port + 5) & 0x20) == 0 );
        outb(port, '0' + i);
    }
}
