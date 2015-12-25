#ifndef SCREEN_H_
#define SCREEN_H_

#include <stdint.h>

namespace os
{

class Screen
{
public:
    enum class EColor
    {
        eBlack        =  0,
        eBlue         =  1,
        eGreen        =  2,
        eCyan         =  3,
        eRed          =  4,
        eMagenta      =  5,
        eBrown        =  6,
        eLightGray    =  7,
        eDarkGray     =  8,
        eLightBlue    =  9,
        eLightGreen   = 10,
        eLightCyan    = 11,
        eLightRed     = 12,
        eLightMagenta = 13,
        eLightBrown   = 14,
        eWhite        = 15,
    };

    Screen();

    void setForegroundColor(EColor color);

    void setBackgroundColor(EColor color);

    void write(char ch);

    void write(const char* str);

    void clear();

private:
    static const int SCREEN_WIDTH;
    static const int SCREEN_HEIGHT;

    uint16_t* textMem;
    uint16_t attrib;
    int csrX;
    int csrY;

    void outputChar(char ch);

    void portWrite(uint16_t port, uint8_t value);

    void updateCursor();

    void scroll();
};

} // namespace os

#endif // SCREEN_H_
