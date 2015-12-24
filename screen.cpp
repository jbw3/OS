#include "screen.h"

namespace os
{

const int Screen::SCREEN_WIDTH = 80;
const int Screen::SCREEN_HEIGHT = 25;

Screen::Screen()
{
    textMem = (uint16_t*)(0xB8000);
    csrX = 0;
    csrY = 0;

    setBackgroundColor(EColor::eBlack);
    setForegroundColor(EColor::eWhite);
}

void Screen::setForegroundColor(EColor color)
{
    uint16_t temp = static_cast<uint16_t>(color);
    temp <<= 8;
    attrib &= 0xF000; // clear all but backgrund color
    attrib |= temp; // set new foreground color
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
    if (ch == '\n')
    {
        ++csrY;
        csrX = 0;
    }
    else if (ch == '\r')
    {
        csrX = 0;
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
        write(' ');
    }

    // reset cursor to the top left corner of the screen
    csrX = 0;
    csrY = 0;
}

void Screen::portWrite(uint16_t port, uint8_t value)
{
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

} // namespace os
