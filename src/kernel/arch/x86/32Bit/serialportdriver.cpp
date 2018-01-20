#include "screen.h"
#include "serialportdriver.h"
#include "system.h"

SerialPortDriver::SerialPortDriver(uint16_t portAddr)
{
    port = portAddr;

    outb(port + LCR, 0);    // disable DLAB
    outb(port + IER, 0);    // disable interrupts
    outb(port + LCR, DLAB); // enable DLAB
    outb(port + DLL, 1);    // set baud divisor to 1 (115,200) (low byte)
    outb(port + DLH, 0);    // (high byte)
    outb(port + LCR, 0x03); // 8 bits, no parity, 1 stop bit
    outb(port + FCR, 0x87); // enable FIFOs, clear them, 8-byte trigger threshold
    outb(port + MCR, 0x0b); // enable IRQs, set RTS/DTR
}

void SerialPortDriver::read(char* buff, size_t nbyte)
{
    for (size_t i = 0; i < nbyte; ++i)
    {
        // wait until data is ready to be read
        while ( (inb(port + LSR) & DATA_READY) == 0 );

        buff[i] = inb(port + RBR);
    }
}

void SerialPortDriver::write(const char* buff, size_t nbyte)
{
    for (size_t i = 0; i < nbyte; ++i)
    {
        // wait until data is ready to be transmitted
        while ( (inb(port + LSR) & EMPTY_TRANS_HOLD_REG) == 0 );

        outb(port + THR, buff[i]);
    }
}
