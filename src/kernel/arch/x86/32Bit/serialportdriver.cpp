#include "irq.h"
#include "serialportdriver.h"
#include "system.h"

SerialPortDriver* SerialPortDriver::instances[] = {nullptr, nullptr, nullptr, nullptr};
unsigned int SerialPortDriver::numInstances = 0;

SerialPortDriver::SerialPortDriver(uint16_t portAddr, unsigned int baudRate)
{
    init();
    if (numInstances >= MAX_NUM_INSTANCES)
    {
        PANIC("Exceeded max number of SerialPortDriver instances.");
    }
    instances[numInstances++] = this;

    // save port
    port = portAddr;

    // calculate baud divisor
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
    outb(port + IER, 0x03); // enable interrupts
}

SerialPortDriver::~SerialPortDriver()
{
    for (unsigned int i = 0; i < numInstances; ++i)
    {
        if (instances[i] == this)
        {
            instances[i] = instances[numInstances - 1];
            instances[numInstances - 1] = nullptr;
            --numInstances;
            break;
        }
    }
}

void SerialPortDriver::read(char* buff, size_t nbyte)
{
    size_t index = 0;
    size_t numToRead = nbyte;
    while (index < nbyte)
    {
        uint8_t* ptr = reinterpret_cast<uint8_t*>(buff + index);
        size_t num = inQ.dequeue(ptr, numToRead);
        index += num;
        numToRead -= num;

        if ( num == 0 && (inb(port + LSR) & 1) != 0 )
        {
            ptr[0] = inb(port + RBR);
            ++index;
            --numToRead;
        }
    }
}

void SerialPortDriver::write(const char* buff, size_t nbyte)
{
    size_t index = 0;
    size_t numToWrite = nbyte;
    while (index < nbyte)
    {
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(buff + index);
        size_t num = outQ.enqueue(ptr, numToWrite);
        index += num;
        numToWrite -= num;

        // if the output reg is empty, write a byte
        if ( (inb(port + LSR) & EMPTY_TRANS_HOLD_REG) != 0 )
        {
            uint8_t value = 0;
            bool avail = outQ.dequeue(value);
            if (avail)
            {
                outb(port + THR, value);
            }
        }
    }
}

void SerialPortDriver::init()
{
    static bool doneInit = false;
    if (!doneInit)
    {
        doneInit = true;

        // register interrupt handler
        registerIrqHandler(IRQ_COM1, interruptHandler);
        registerIrqHandler(IRQ_COM2, interruptHandler);
    }
}

void SerialPortDriver::interruptHandler(const registers* /*regs*/)
{
    for (unsigned int i = 0; i < numInstances; ++i)
    {
        SerialPortDriver* instance = instances[i];
        instance->processInterrupt();
    }
}

void SerialPortDriver::processInterrupt()
{
    uint8_t iirVal = inb(port + IIR);
    uint8_t intType = iirVal & INT_TYPE_MASK;

    if ( (iirVal & NO_PENDING_INT) == 0 )
    {
        if (intType == INT_RECEIVE_AVAIL || intType == INT_TIME_OUT_PENDING)
        {
            uint8_t value = inb(port + RBR);
            inQ.enqueue(value);
        }
        else if (intType == INT_TRANS_EMPTY)
        {
            uint8_t value = 0;
            bool avail = outQ.dequeue(value);
            if (avail)
            {
                outb(port + THR, value);
            }
        }
    }
}
