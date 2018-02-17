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
    stream = nullptr;

    qHead = 0;
    qTail = 0;

    base = 10;
    flags = BOOL_ALPHA;
    width = 0;
    fill = ' ';
    precision = 10;
}

void Screen::setStream(VgaDriver* streamPtr)
{
    stream = streamPtr;
}

VgaDriver::EColor Screen::getForegroundColor() const
{
    return stream->getForegroundColor();
}

void Screen::setForegroundColor(VgaDriver::EColor color)
{
    stream->setForegroundColor(color);
}

VgaDriver::EColor Screen::getBackgroundColor() const
{
    return stream->getBackgroundColor();
}

void Screen::setBackgroundColor(VgaDriver::EColor color)
{
    stream->setBackgroundColor(color);
}

void Screen::write(char ch)
{
    if (width > 0)
    {
        justify(1);
    }
    stream->write(reinterpret_cast<const uint8_t*>(&ch), 1);
}

void Screen::write(const char* str)
{
    size_t len = strlen(str);

    if (width > 0)
    {
        justify(len);
    }

    stream->write(reinterpret_cast<const uint8_t*>(str), len);
}

void Screen::clear()
{
    int width = 80;
    int height = 25;

    // clear the screen by filling it with spaces
    stream->setCursorX(0);
    stream->setCursorY(0);
    uint8_t ch = ' ';
    for (int i = 0; i < width * height; ++i)
    {
        stream->write(&ch, 1);
    }

    // reset cursor to the top left corner of the screen
    stream->setCursorX(0);
    stream->setCursorY(0);
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

void Screen::justify(size_t strLen)
{
    for (size_t i = strLen; i < width; ++i)
    {
        stream->write(reinterpret_cast<const uint8_t*>(&fill), 1);
    }

    width = 0;
}

} // namespace os

// create instance of screen
os::Screen screen;
