#include "screen.h"
#include "serialportdriver.h"
#include "system.h"

SerialPortDriver::SerialPortDriver(uint16_t portAddr, unsigned int baudRate)
{
    port = portAddr;

    unsigned int baudDivisor = 1;
    if (baudRate > 0)
    {
        baudDivisor = CLOCK_RATE / baudRate;
        if (baudDivisor == 0)
        {
            // 0 is invalid. Instead, set it to the highest baud rate.
            baudDivisor = 1;
        }
    }
    else
    {
        baudDivisor = 1;
    }

    uint8_t baudDivisorLo = (baudDivisor & 0x00ff);
    uint8_t baudDivisorHi = (baudDivisor & 0xff00) >> 8;

    outb(port + LCR, 0);    // disable DLAB
    outb(port + IER, 0);    // disable interrupts
    outb(port + LCR, DLAB); // enable DLAB
    outb(port + DLL, baudDivisorLo); // set baud divisor low byte
    outb(port + DLH, baudDivisorHi); // set baud divisor high byte
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
