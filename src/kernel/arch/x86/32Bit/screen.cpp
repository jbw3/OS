#include <string.h>

#include "screen.h"
#include "system.h"

/// @todo JBW - This header file should not be
/// referenced directly. Temporarily doing it
/// until the Screen class is replaced by ostream
#include "../../../../libs/c/src/stringutils.h"

namespace
{

template<typename T>
const T FLOAT_MAX_ORDER = 1;


/// @todo temporary max order
template<>
const float FLOAT_MAX_ORDER<float> = 1e20f;

/// @todo temporary max order
template<>
const double FLOAT_MAX_ORDER<double> = 1e100;

/// @todo temporary max order
template<>
const long double FLOAT_MAX_ORDER<long double> = 1e200l;

/// @todo handle infinity and NaN
/// @todo move this to stringutils
template<typename T>
void writeFixedFloat(T num, char* str, unsigned int precision)
{
    int idx = 0;

    if (num < static_cast<T>(0))
    {
        str[idx++] = '-';
        num = -num;
    }

    T order = FLOAT_MAX_ORDER<T>;

    bool outputPoint = false;
    bool outputZeros = false;
    unsigned int fractionDigits = 0;
    while (fractionDigits < precision)
    {
        /// @todo Check order/10. If > 7, subtract 1 from that order
        T result = num / order;
        result += static_cast<T>(0.001); /// @todo this sometimes causes incorrect behavior (e.g. 999.999)
        int digit = static_cast<int>(result);

        if (outputZeros || digit != 0)
        {
            char ch = '0' + digit;
            str[idx++] = ch;
            outputZeros = true;
        }

        if (outputPoint)
        {
            ++fractionDigits;
        }

        T sub = digit * order;
        num -= sub;
        if (num < static_cast<T>(0))
        {
            num = static_cast<T>(0);
        }

        order /= static_cast<T>(10);
        if (!outputPoint && order < static_cast<T>(1))
        {
            if (!outputZeros)
            {
                str[idx++] = '0';
                outputZeros = true;
            }
            str[idx++] = '.';
            outputPoint = true;
        }
    }

    str[idx] = '\0';
}

} // anonymous namespace

namespace os
{

const int Screen::SCREEN_WIDTH = 80;
const int Screen::SCREEN_HEIGHT = 25;
const int Screen::TAB_SIZE = 4;

const uint8_t Screen::BOOL_ALPHA = 0x01;
const uint8_t Screen::UPPERCASE  = 0x02;

Screen& Screen::bin(Screen& s)
{
    s.base = 2;
    return s;
}

Screen& Screen::oct(Screen& s)
{
    s.base = 8;
    return s;
}

Screen& Screen::dec(Screen& s)
{
    s.base = 10;
    return s;
}

Screen& Screen::hex(Screen& s)
{
    s.base = 16;
    return s;
}

Screen& Screen::boolalpha(Screen& s)
{
    s.flags |= BOOL_ALPHA;
    return s;
}

Screen& Screen::noboolalpha(Screen& s)
{
    s.flags &= ~BOOL_ALPHA;
    return s;
}

Screen& Screen::uppercase(Screen& s)
{
    s.flags |= UPPERCASE;
    return s;
}

Screen& Screen::nouppercase(Screen& s)
{
    s.flags &= ~UPPERCASE;
    return s;
}

Screen::Manip<char> Screen::setfill(char ch)
{
    return Manip<char>(setFill, ch);
}

Screen::Manip<size_t> Screen::setw(size_t width)
{
    return Manip<size_t>(setWidth, width);
}

Screen::Screen()
{
    textMem = (uint16_t*)(0xB8000 + KERNEL_VIRTUAL_BASE);
    csrX = 0;
    csrY = 0;

    qHead = 0;
    qTail = 0;

    base = 10;
    flags = BOOL_ALPHA;
    width = 0;
    fill = ' ';
    precision = 10;

    // disable blinking
    setBlinking(false);

    setBackgroundColor(EColor::eBlack);
    setForegroundColor(EColor::eWhite);
}

Screen::EColor Screen::getForegroundColor() const
{
    uint16_t color = 0x0F00 & attrib;
    color >>= 8;
    return static_cast<EColor>(color);
}

void Screen::setForegroundColor(EColor color)
{
    uint16_t temp = static_cast<uint16_t>(color);
    temp <<= 8;
    attrib &= 0xF000; // clear all but background color
    attrib |= temp; // set new foreground color
}

Screen::EColor Screen::getBackgroundColor() const
{
    uint16_t color = 0xF000 & attrib;
    color >>= 12;
    return static_cast<EColor>(color);
}

void Screen::setBackgroundColor(EColor color)
{
    uint16_t temp = static_cast<uint16_t>(color);
    temp <<= 12;
    attrib &= 0x0F00; // clear all but foreground color
    attrib |= temp; // set new background color
}

void Screen::setBlinking(bool enabled)
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

void Screen::write(char ch)
{
    if (width > 0)
    {
        justify(1);
    }
    rawWrite(ch);
}

void Screen::write(const char* str)
{
    if (width > 0)
    {
        justify(strlen(str));
    }

    size_t idx = 0;
    char ch = str[idx];
    while (ch != '\0')
    {
        rawWrite(ch);
        ++idx;
        ch = str[idx];
    }
}

void Screen::clear()
{
    // clear the screen by filling it with spaces
    csrX = 0;
    csrY = 0;
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i)
    {
        outputChar(' ');
    }

    // reset cursor to the top left corner of the screen
    csrX = 0;
    csrY = 0;

    updateCursor();
}

Screen& Screen::operator <<(char ch)
{
    write(ch);
    return *this;
}

Screen& Screen::operator <<(const char* str)
{
    write(str);
    return *this;
}

Screen& Screen::operator <<(bool b)
{
    if ((flags & BOOL_ALPHA) != 0)
    {
        write(b ? "true" : "false");
    }
    else
    {
        write(b ? '1' : '0');
    }
    return *this;
}

Screen& Screen::operator <<(signed char num)
{
    char buff[sizeof(num) + 2]; // sizeof(num) + negative sign + null
    writeSignedNum(num, buff, base, (flags & UPPERCASE));
    write(buff);
    return *this;
}

Screen& Screen::operator <<(short num)
{
    char buff[sizeof(num) + 2]; // sizeof(num) + negative sign + null
    writeSignedNum(num, buff, base, (flags & UPPERCASE));
    write(buff);
    return *this;
}

Screen& Screen::operator <<(int num)
{
    char buff[sizeof(num) + 2]; // sizeof(num) + negative sign + null
    writeSignedNum(num, buff, base, (flags & UPPERCASE));
    write(buff);
    return *this;
}

Screen& Screen::operator <<(long num)
{
    char buff[sizeof(num) + 2]; // sizeof(num) + negative sign + null
    writeSignedNum(num, buff, base, (flags & UPPERCASE));
    write(buff);
    return *this;
}

Screen& Screen::operator <<(long long num)
{
    char buff[sizeof(num) + 2]; // sizeof(num) + negative sign + null
    writeSignedNum(num, buff, base, (flags & UPPERCASE));
    write(buff);
    return *this;
}

Screen& Screen::operator <<(unsigned char num)
{
    char buff[sizeof(num) + 2]; // sizeof(num) + negative sign + null
    writeUnsignedNum(num, buff, base, (flags & UPPERCASE));
    write(buff);
    return *this;
}

Screen& Screen::operator <<(unsigned short num)
{
    char buff[sizeof(num) + 2]; // sizeof(num) + negative sign + null
    writeUnsignedNum(num, buff, base, (flags & UPPERCASE));
    write(buff);
    return *this;
}

Screen& Screen::operator <<(unsigned int num)
{
    char buff[sizeof(num) + 2]; // sizeof(num) + negative sign + null
    writeUnsignedNum(num, buff, base, (flags & UPPERCASE));
    write(buff);
    return *this;
}

Screen& Screen::operator <<(unsigned long num)
{
    char buff[sizeof(num) + 2]; // sizeof(num) + negative sign + null
    writeUnsignedNum(num, buff, base, (flags & UPPERCASE));
    write(buff);
    return *this;
}

Screen& Screen::operator <<(unsigned long long num)
{
    char buff[sizeof(num) + 2]; // sizeof(num) + negative sign + null
    writeUnsignedNum(num, buff, base, (flags & UPPERCASE));
    write(buff);
    return *this;
}

Screen& Screen::operator <<(const void* ptr)
{
    char buff[sizeof(ptr) + 2]; // sizeof(ptr) + negative sign + null
    writeUnsignedNum(reinterpret_cast<uintptr_t>(ptr), buff, base, (flags & UPPERCASE));
    write(buff);
    return *this;
}

Screen& Screen::operator <<(float num)
{
    /// @todo calculate max buffer size
    char buff[128];
    writeFixedFloat(num, buff, precision);
    write(buff);
    return *this;
}

Screen& Screen::operator <<(double num)
{
    /// @todo calculate max buffer size
    char buff[128];
    writeFixedFloat(num, buff, precision);
    write(buff);
    return *this;
}

Screen& Screen::operator <<(long double num)
{
    /// @todo calculate max buffer size
    char buff[128];
    writeFixedFloat(num, buff, precision);
    write(buff);
    return *this;
}

Screen& Screen::operator <<(Screen& (*fPtr)(Screen&))
{
    return fPtr(*this);
}

void Screen::setFill(Screen& s, char ch)
{
    s.fill = ch;
}

void Screen::setWidth(Screen& s, size_t width)
{
    s.width = width;
}

void Screen::outputChar(char ch)
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
        int spaces = TAB_SIZE - (csrX % TAB_SIZE);
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

        int offset = csrY * SCREEN_WIDTH + csrX;
        *(textMem + offset) = val;

        ++csrX;
        if (csrX >= SCREEN_WIDTH)
        {
            ++csrY;
            csrX = 0;
        }
    }
}

void Screen::updateCursor()
{
    int pos = csrY * SCREEN_WIDTH + csrX;

    // set the upper and lower bytes of the
    // blinking cursor index
    outb(0x3D4, 14);
    outb(0x3D5, (uint8_t)(pos >> 8));
    outb(0x3D4, 15);
    outb(0x3D5, (uint8_t)(pos));
}

void Screen::scroll()
{
    if (csrY >= SCREEN_HEIGHT)
    {
        // move all lines up by 1
        for (int line = 1; line < SCREEN_HEIGHT; ++line)
        {
            int dstIdx = (line - 1) * SCREEN_WIDTH;
            int srcIdx = line * SCREEN_WIDTH;
            memcpy(textMem + dstIdx, textMem + srcIdx, SCREEN_WIDTH * sizeof(uint16_t));
        }

        // clear bottom line
        csrX = 0;
        csrY = SCREEN_HEIGHT - 1;
        for (int x = 0; x < SCREEN_WIDTH; ++x)
        {
            outputChar(' ');
        }

        // reset cursor to the bottom left corner of the screen
        csrX = 0;
        csrY = SCREEN_HEIGHT - 1;
    }
}

void Screen::rawWrite(char ch)
{
    outputChar(ch);
    scroll();
    updateCursor();
}

void Screen::justify(size_t strLen)
{
    for (size_t i = strLen; i < width; ++i)
    {
        rawWrite(fill);
    }

    width = 0;
}

} // namespace os

// create instance of screen
os::Screen screen;
