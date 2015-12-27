#include <string.h>
#include "screen.h"

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

void Screen::portWrite(uint16_t port, uint8_t value)
{
    asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

void Screen::updateCursor()
{
    int pos = csrY * SCREEN_WIDTH + csrX;

    // set the upper and lower bytes of the
    // blinking cursor index
    portWrite(0x3D4, 14);
    portWrite(0x3D5, (uint8_t)(pos >> 8));
    portWrite(0x3D4, 15);
    portWrite(0x3D5, (uint8_t)(pos));
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

} // namespace os

// create instance of screen
os::Screen screen;
