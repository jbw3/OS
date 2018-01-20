#ifndef SERIAL_PORT_DRIVER_H_
#define SERIAL_PORT_DRIVER_H_

#include <unistd.h>
#include <stdint.h>

class SerialPortDriver
{
public:
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

    /// UART clock rate
    static constexpr unsigned int CLOCK_RATE = 115'200;

    SerialPortDriver(uint16_t portAddr, unsigned int baudRate);

    void read(char* buff, size_t nbyte);

    void write(const char* buff, size_t nbyte);

private:
    uint16_t port;
};

#endif // SERIAL_PORT_DRIVER_H_
