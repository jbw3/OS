#ifndef VGA_DRIVER_H_
#define VGA_DRIVER_H_

#include "stream.h"

class VgaDriver : public Stream
{
public:
    static constexpr int SCREEN_WIDTH = 80;
    static constexpr int SCREEN_HEIGHT = 25;
    static constexpr int TAB_SIZE = 4;

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

    VgaDriver();

    EColor getForegroundColor() const;

    void setForegroundColor(EColor color);

    EColor getBackgroundColor() const;

    void setBackgroundColor(EColor color);

    void setBlinking(bool enabled);

    void setCursorX(int x);

    void setCursorY(int y);

    bool canRead() const override
    {
        return false;
    }

    bool canWrite() const override
    {
        return true;
    }

    ssize_t read(uint8_t*, size_t) override
    {
        return -1;
    }

    ssize_t write(const uint8_t* buff, size_t nbyte) override;

private:
    uint16_t* textMem;
    uint16_t attrib;
    int csrX;
    int csrY;

    void writeChar(char ch);

    void outputChar(char ch);

    void scroll();

    void updateCursor();
};

#endif // VGA_DRIVER_H_
