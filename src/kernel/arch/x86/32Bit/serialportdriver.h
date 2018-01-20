#ifndef SERIAL_PORT_DRIVER_H_
#define SERIAL_PORT_DRIVER_H_

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

    /// Divisor Latch Access Bit
    static constexpr uint8_t DLAB = 0x80;

    SerialPortDriver(uint16_t portAddr);

private:
    uint16_t port;
};

#endif // SERIAL_PORT_DRIVER_H_
