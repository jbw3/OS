#include <string.h>
#include "screen.h"
#include "system.h"

namespace os
{

const int Screen::SCREEN_WIDTH = 80;
const int Screen::SCREEN_HEIGHT = 25;
const int Screen::TAB_SIZE = 4;

Screen::Screen()
{
}

void Screen::init()
{
    textMem = (uint16_t*)(0xB8000);
    csrX = 0;
    csrY = 0;

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

void Screen::write(char ch)
{
    outputChar(ch);
    scroll();
    updateCursor();
}

void Screen::write(const char* str)
{
    unsigned long idx = 0;
    char ch = str[idx];
    while (ch != '\0')
    {
        write(ch);
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
    write(b ? '1' : '0');
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

template<typename T>
void Screen::writeSigned(T num)
{
    // need 19 chars for max signed 64-bit decimal number (9,223,372,036,854,775,807)
    // and 1 char for possible negative sign
    char buff[20];

    int idx = 0;

    if (num < 0)
    {
        write('-');
    }
    else
    {
        num = -num;
    }

    do
    {
        char digit = -(num % 10);
        buff[idx++] = digit + '0';
        num /= 10;
    } while (num < 0);

    while (idx > 0)
    {
        --idx;
        write(buff[idx]);
    }
}

template<typename T>
void Screen::writeUnsigned(T num)
{
    // need 20 chars for max unsigned 64-bit decimal number (18,446,744,073,709,551,615)
    char buff[20];

    int idx = 0;

    do
    {
        char digit = num % 10;
        buff[idx++] = digit + '0';
        num /= 10;
    } while (num > 0);

    while (idx > 0)
    {
        --idx;
        write(buff[idx]);
    }
}

} // namespace os

// create instance of screen
os::Screen screen;
