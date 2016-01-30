#include <string.h>
#include "screen.h"
#include "system.h"

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
    textMem = (uint16_t*)(0xB8000);
    csrX = 0;
    csrY = 0;

    qHead = 0;
    qTail = 0;

    base = 10;
    flags = BOOL_ALPHA;
    width = 0;
    fill = ' ';

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

void Screen::addInput(char ch)
{
    // the char to the queue if it is not full
    if ( (qTail != qHead - 1) && !(qHead == 0 && qTail == IN_QUEUE_SIZE - 1) )
    {
        inQueue[qTail] = ch;
        if (qTail >= IN_QUEUE_SIZE - 1)
        {
            qTail = 0;
        }
        else
        {
            ++qTail;
        }
    }
}

bool Screen::read(char& ch)
{
    // if the queue is empty return false
    if (qHead == qTail)
    {
        return false;
    }

    ch = inQueue[qHead];
    if (qHead >= IN_QUEUE_SIZE - 1)
    {
        qHead = 0;
    }
    else
    {
        ++qHead;
    }

    return true;
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
    writeSigned(num);
    return *this;
}

Screen& Screen::operator <<(short num)
{
    writeSigned(num);
    return *this;
}

Screen& Screen::operator <<(int num)
{
    writeSigned(num);
    return *this;
}

Screen& Screen::operator <<(long num)
{
    writeSigned(num);
    return *this;
}

Screen& Screen::operator <<(long long num)
{
    writeSigned(num);
    return *this;
}

Screen& Screen::operator <<(unsigned char num)
{
    writeUnsigned(num);
    return *this;
}

Screen& Screen::operator <<(unsigned short num)
{
    writeUnsigned(num);
    return *this;
}

Screen& Screen::operator <<(unsigned int num)
{
    writeUnsigned(num);
    return *this;
}

Screen& Screen::operator <<(unsigned long num)
{
    writeUnsigned(num);
    return *this;
}

Screen& Screen::operator <<(unsigned long long num)
{
    writeUnsigned(num);
    return *this;
}

Screen& Screen::operator <<(const void* ptr)
{
    writeUnsigned((uintptr_t)ptr);
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

template<typename T>
void Screen::writeSigned(T num)
{
    // need 64 chars for max signed 64-bit number,
    // 1 char for possible negative sign,
    // and 1 char for null
    char buff[66];
    buff[65] = '\0';

    int idx = 65;
    bool negative = true;
    if (num >= 0)
    {
        negative = false;
        num = -num;
    }

    do
    {
        char digit = -(num % base);
        digitToChar(digit);
        buff[--idx] = digit;
        num /= base;
    } while (num < 0);

    if (negative)
    {
        buff[--idx] = '-';
    }

    write(buff + idx);
}

template<typename T>
void Screen::writeUnsigned(T num)
{
    // need 64 chars for max unsigned 64-bit number
    // and 1 char for null
    char buff[65];
    buff[64] = '\0';

    int idx = 64;

    do
    {
        char digit = num % base;
        digitToChar(digit);
        buff[--idx] = digit;
        num /= base;
    } while (num > 0);


    write(buff + idx);
}

void Screen::digitToChar(char& digit)
{
    if (digit >= 10)
    {
        if ((flags & UPPERCASE) != 0)
        {
            digit += 'A' - 10;
        }
        else
        {
            digit += 'a' - 10;
        }
    }
    else
    {
        digit += '0';
    }
}

} // namespace os

// create instance of screen
os::Screen screen;
