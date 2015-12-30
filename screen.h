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

    void init();

    EColor getForegroundColor() const;

    void setForegroundColor(EColor color);

    EColor getBackgroundColor() const;

    void setBackgroundColor(EColor color);

    void write(char ch);

    void write(const char* str);

    void clear();

    os::Screen& operator <<(char ch);

    os::Screen& operator <<(const char* str);

    os::Screen& operator <<(bool b);

    os::Screen& operator <<(signed char num);

    os::Screen& operator <<(short num);

    os::Screen& operator <<(int num);

    os::Screen& operator <<(long num);

    os::Screen& operator <<(unsigned char num);

    os::Screen& operator <<(unsigned short num);

    os::Screen& operator <<(unsigned int num);

    os::Screen& operator <<(unsigned long num);

private:
    static const int SCREEN_WIDTH;
    static const int SCREEN_HEIGHT;
    static const int TAB_SIZE;

    uint16_t* textMem;
    uint16_t attrib;
    int csrX;
    int csrY;

    void outputChar(char ch);

    void updateCursor();

    void scroll();

    template<typename T>
    void writeSigned(T num);

    template<typename T>
    void writeUnsigned(T num);
};

} // namespace os

extern os::Screen screen;

#endif // SCREEN_H_
