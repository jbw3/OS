#ifndef SERIAL_PORT_DRIVER_H_
#define SERIAL_PORT_DRIVER_H_

#include <unistd.h>
#include <stdint.h>

#include "queue.hpp"
#include "stream.h"

struct registers;

class SerialPortDriver : public Stream
{
public:
    /// COM1 port address
    static constexpr uint16_t COM1_PORT = 0x3F8;

    /// COM2 port address
    static constexpr uint16_t COM2_PORT = 0x2F8;

    /// COM3 port address
    static constexpr uint16_t COM3_PORT = 0x3E8;

    /// COM4 port address
    static constexpr uint16_t COM4_PORT = 0x2E8;

    /// Transmit holding buffer offset
    static constexpr uint8_t THR = 0;

    /// Receiver buffer offset
    static constexpr uint8_t RBR = 0;

    /// Divisor latch low byte offset
    static constexpr uint8_t DLL = 0;

    /// Interrupt enable register offset
    static constexpr uint8_t IER = 1;

    /// Divisor latch high byte offset
    static constexpr uint8_t DLH = 1;

    /// Interrupt identification register offset
    static constexpr uint8_t IIR = 2;

    /// FIFO control register offset
    static constexpr uint8_t FCR = 2;

    /// Line control register offset
    static constexpr uint8_t LCR = 3;

    /// Modem control register offset
    static constexpr uint8_t MCR = 4;

    /// Line status register offset
    static constexpr uint8_t LSR = 5;

    /// Modem status register offset
    static constexpr uint8_t MSR = 6;

    /// Scratch register offset
    static constexpr uint8_t SR  = 7;

    /// Divisor Latch Access Bit
    static constexpr uint8_t DLAB = 0x80;

    /// Data is ready to be read
    static constexpr uint8_t DATA_READY = 0x01;

    /// Data can be transmitted
    static constexpr uint8_t EMPTY_TRANS_HOLD_REG = 0x20;

    /// Interrupt identification register interrupt type mask
    static constexpr uint8_t INT_TYPE_MASK = 0xE;

    /// Modem status interrupt
    static constexpr uint8_t INT_MODEM_STATUS = 0x0;

    /// Transmitter holding register empty interrupt
    static constexpr uint8_t INT_TRANS_EMPTY = 0x2;

    /// Received data available interrupt
    static constexpr uint8_t INT_RECEIVE_AVAIL = 0x4;

    /// receiver line status interrupt
    static constexpr uint8_t INT_RECEIVER_LINE_STAT = 0x7;

    /// Time-out interrupt pending
    static constexpr uint8_t INT_TIME_OUT_PENDING = 0xC;

    /// No interrupt is pending
    static constexpr uint8_t NO_PENDING_INT = 0x01;

    /// UART clock rate
    static constexpr unsigned int CLOCK_RATE = 115'200;

    SerialPortDriver(uint16_t portAddr, unsigned int baudRate);

    ~SerialPortDriver();

    bool canRead() const override
    {
        return true;
    }

    bool canWrite() const override
    {
        return true;
    }

    ssize_t read(uint8_t* buff, size_t nbyte) override;

    ssize_t write(const uint8_t* buff, size_t nbyte) override;

    void flush() override;

private:
    static constexpr unsigned int MAX_NUM_INSTANCES = 4;
    static SerialPortDriver* instances[MAX_NUM_INSTANCES];
    static unsigned int numInstances;

    uint16_t port;
    Queue<uint8_t, 64> inQ;
    Queue<uint8_t, 64> outQ;

    static void init();

    static void interruptHandler(const registers* regs);

    void processInterrupt();
};

#endif // SERIAL_PORT_DRIVER_H_
