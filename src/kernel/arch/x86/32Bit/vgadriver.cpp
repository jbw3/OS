#include <ctype.h>
#include <string.h>

#include "vgadriver.h"
#include "system.h"

VgaDriver::VgaDriver()
{
    textMem = reinterpret_cast<uint16_t*>(0xB8000 + KERNEL_VIRTUAL_BASE);
    csrX = 0;
    csrY = 0;
    inEscSequence = false;
    escSequenceChar = '\0';
    csiState = eParameter;
    parameterBytesSize = 0;
    intermediateBytesSize = 0;

    // disable blinking
    setBlinking(false);

    // sets attrib
    setBackgroundColor(EColor::eBlack);
    setForegroundColor(EColor::eWhite);
}

VgaDriver::EColor VgaDriver::getForegroundColor() const
{
    uint16_t color = 0x0F00 & attrib;
    color >>= 8;
    return static_cast<EColor>(color);
}

void VgaDriver::setForegroundColor(EColor color)
{
    uint16_t temp = static_cast<uint16_t>(color);
    temp <<= 8;
    attrib &= 0xF000; // clear all but background color
    attrib |= temp; // set new foreground color
}

VgaDriver::EColor VgaDriver::getBackgroundColor() const
{
    uint16_t color = 0xF000 & attrib;
    color >>= 12;
    return static_cast<EColor>(color);
}

void VgaDriver::setBackgroundColor(EColor color)
{
    uint16_t temp = static_cast<uint16_t>(color);
    temp <<= 12;
    attrib &= 0x0F00; // clear all but foreground color
    attrib |= temp; // set new background color
}

void VgaDriver::setBlinking(bool enabled)
{
    constexpr uint16_t ADDR_REG = 0x3C0; // attribute address/data register
    constexpr uint16_t DATA_REG = 0x3C1; // attribute data read register

    constexpr uint8_t ATT_MODE_CTRL_REG = 0x10; // attribute mode control register

    constexpr uint8_t PAS_BIT = 0x20; // palette address source
    constexpr uint8_t BLINK_BIT = 0x08; // blink bit: 0 - disabled, 1 - enabled

    // disable interrupts
    clearInt();

    // reset flip-flop
    inb(0x3DA);

    // save previous value
    uint8_t prevAddr = inb(ADDR_REG);

    outb(ADDR_REG, ATT_MODE_CTRL_REG | PAS_BIT);
    uint8_t val = inb(DATA_REG);
    val = enabled ? (val | BLINK_BIT) : (val & ~BLINK_BIT);
    outb(DATA_REG, val);

    // restore previous value
    outb(ADDR_REG, prevAddr | PAS_BIT);

    // re-enable interrupts
    setInt();
}

unsigned int VgaDriver::getCursorX() const
{
    return csrX;
}

void VgaDriver::setCursorX(unsigned int x)
{
    csrX = x;

    if (x >= SCREEN_WIDTH)
    {
        csrX = SCREEN_WIDTH - 1;
    }

    updateCursor();
}

unsigned int VgaDriver::getCursorY() const
{
    return csrY;
}

void VgaDriver::setCursorY(unsigned int y)
{
    csrY = y;

    if (y >= SCREEN_HEIGHT)
    {
        csrY = SCREEN_HEIGHT - 1;
    }

    updateCursor();
}

ssize_t VgaDriver::write(const uint8_t* buff, size_t nbyte)
{
    size_t i = 0;
    for (; i < nbyte; ++i)
    {
        writeChar(buff[i]);
    }

    return static_cast<ssize_t>(i);
}

void VgaDriver::writeChar(char ch)
{
    if (inEscSequence)
    {
        parseEscSequence(ch);
    }
    else if (ch == ESCAPE)
    {
        inEscSequence = true;
    }
    else
    {
        outputChar(ch);
        scroll();
        updateCursor();
    }
}

void VgaDriver::outputChar(char ch)
{
    if (ch == '\n')
    {
        ++csrY;
        csrX = 0;
    }
    else if (ch == '\r')
    {
        csrX = 0;
    }
    else if (ch == '\t')
    {
        unsigned int spaces = TAB_SIZE - (csrX % TAB_SIZE);
        csrX += spaces;
        if (csrX >= SCREEN_WIDTH)
        {
            ++csrY;
            csrX = 0;
        }
    }
    else if (ch == '\b')
    {
        if (csrX > 0)
        {
            --csrX;
        }
    }
    else
    {
        uint16_t val = attrib | ch;

        unsigned int offset = csrY * SCREEN_WIDTH + csrX;
        *(textMem + offset) = val;

        ++csrX;
        if (csrX >= SCREEN_WIDTH)
        {
            ++csrY;
            csrX = 0;
        }
    }
}

void VgaDriver::scroll()
{
    if (csrY >= SCREEN_HEIGHT)
    {
        // move all lines up by 1
        for (unsigned int line = 1; line < SCREEN_HEIGHT; ++line)
        {
            unsigned int dstIdx = (line - 1) * SCREEN_WIDTH;
            unsigned int srcIdx = line * SCREEN_WIDTH;
            memcpy(textMem + dstIdx, textMem + srcIdx, SCREEN_WIDTH * sizeof(uint16_t));
        }

        // clear bottom line
        csrX = 0;
        csrY = SCREEN_HEIGHT - 1;
        for (unsigned int x = 0; x < SCREEN_WIDTH; ++x)
        {
            outputChar(' ');
        }

        // reset cursor to the bottom left corner of the screen
        csrX = 0;
        csrY = SCREEN_HEIGHT - 1;
    }
}

void VgaDriver::updateCursor()
{
    unsigned int pos = csrY * SCREEN_WIDTH + csrX;

    // set the upper and lower bytes of the
    // blinking cursor index
    outb(0x3D4, 14);
    outb(0x3D5, static_cast<uint8_t>(pos >> 8));
    outb(0x3D4, 15);
    outb(0x3D5, static_cast<uint8_t>(pos));
}

void VgaDriver::parseEscSequence(char ch)
{
    if (escSequenceChar == '\0')
    {
        escSequenceChar = ch;
    }
    else if (escSequenceChar == CSI)
    {
        parseCsi(ch);
    }
}

void VgaDriver::parseCsi(char ch)
{
    bool reset = false;

    if (ch >= CSI_START_PARAMETER_BYTE && ch <= CSI_END_PARAMETER_BYTE)
    {
        if (csiState == eParameter)
        {
            if (parameterBytesSize < MAX_PARAMETER_BYTES_SIZE - 1)
            {
                parameterBytes[parameterBytesSize++] = ch;
            }
            else
            {
                // error: ran out of buffer space
                reset = true;
            }
        }
        else
        {
            // error: got a parameter character while not in parameter state
            reset = true;
        }
    }
    else if (ch >= CSI_START_INTERMEDIATE_BYTE && ch <= CSI_END_INTERMEDIATE_BYTE)
    {
        if (csiState == eParameter)
        {
            // go to next state
            csiState = eIntermediate;
        }

        if (csiState == eIntermediate)
        {
            if (intermediateBytesSize < MAX_INTERMEDIATE_BYTES_SIZE - 1)
            {
                intermediateBytes[intermediateBytesSize++] = ch;
            }
            else
            {
                // error: ran out of buffer space
                reset = true;
            }
        }
        else
        {
            // error: got an intermidiate character while not in intermidiate state
            reset = true;
        }
    }
    else if (ch >= CSI_START_FINAL_BYTE && ch <= CSI_END_FINAL_BYTE)
    {
        parameterBytes[parameterBytesSize] = '\0';
        intermediateBytes[intermediateBytesSize] = '\0';
        finalByte = ch;

        evalCsi();

        // done with sequence, reset state
        reset = true;
    }

    if (reset)
    {
        inEscSequence = false;
        escSequenceChar = '\0';
        csiState = eParameter;
        parameterBytesSize = 0;
        intermediateBytesSize = 0;
    }
}

void VgaDriver::evalCsi()
{
    const char* ptr = nullptr;
    bool error = false;

    if (finalByte == 'A')
    {
        unsigned int y = 0;
        bool done = getNumParam(y, 1, error, ptr);
        if (done && !error)
        {
            setCursorY(csrY - y);
        }
    }
    else if (finalByte == 'B')
    {
        unsigned int y = 0;
        bool done = getNumParam(y, 1, error, ptr);
        if (done && !error)
        {
            setCursorY(csrY + y);
        }
    }
    else if (finalByte == 'C')
    {
        unsigned int x = 0;
        bool done = getNumParam(x, 1, error, ptr);
        if (done && !error)
        {
            setCursorX(csrX + x);
        }
    }
    else if (finalByte == 'D')
    {
        unsigned int x = 0;
        bool done = getNumParam(x, 1, error, ptr);
        if (done && !error)
        {
            setCursorX(csrX - x);
        }
    }
    else if (finalByte == 'E')
    {
        unsigned int y = 0;
        bool done = getNumParam(y, 1, error, ptr);
        if (done && !error)
        {
            setCursorX(0);
            setCursorY(csrY + y);
        }
    }
    else if (finalByte == 'F')
    {
        unsigned int y = 0;
        bool done = getNumParam(y, 1, error, ptr);
        if (done && !error)
        {
            setCursorX(0);
            setCursorY(csrY - y);
        }
    }
    else if (finalByte == 'G')
    {
        unsigned int x = 0;
        bool done = getNumParam(x, 1, error, ptr);
        if (done && !error)
        {
            if (x > 0)
            {
                // convert the escape sequence's 1-based index
                // to a 0-based index
                --x;
            }
            setCursorX(x);
        }
    }
}

bool VgaDriver::getNumParam(unsigned int& num, unsigned int def, bool& error, const char*& ptr)
{
    // (MAX_PARAM * 10) must fit in an unsigned int (32-bits) for
    // the check below to work
    constexpr unsigned int MAX_PARAM = 100'000'000;

    // if this is the first time this function is called,
    // start at the beginning of the paramter string
    if (ptr == nullptr)
    {
        ptr = parameterBytes;
    }

    // if there is no number before the delimiter, use the
    // default value
    if (*ptr == ';' || *ptr == '\0')
    {
        num = def;
        error = false;
        // we're done if this is the end of the string
        return (*ptr == '\0');
    }

    num = 0;
    while (*ptr != '\0')
    {
        if (isdigit(*ptr))
        {
            num *= 10;
            if (num > MAX_PARAM)
            {
                error = true;
                return true;
            }

            num += *ptr - '0';
        }
        else if (*ptr == ';')
        {
            // increment the pointer for the next time this function is called
            ++ptr;
            error = false;
            return false;
        }
        else // invalid character
        {
            num = 0;
            error = true;
            return true;
        }

        ++ptr;
    }

    error = false;
    // when we get here, we're done parsing the string
    return true;
}
