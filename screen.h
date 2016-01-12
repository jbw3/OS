#ifndef SCREEN_H_
#define SCREEN_H_

#include <stddef.h>
#include <stdint.h>

namespace os
{

// forward declaration
class Screen;

class Screen
{
private:
    template<typename T>
    class Manip
    {
    public:
        Manip(void(*funcPtr)(os::Screen& s, T arg), T argument)
        {
            fPtr = funcPtr;
            arg = argument;
        }

        void exec(os::Screen& s)
        {
            fPtr(s, arg);
        }

    private:
        void(*fPtr)(os::Screen& s, T arg);
        T arg;
    };

public:
    static os::Screen& bin(os::Screen& s);

    static os::Screen& oct(os::Screen& s);

    static os::Screen& dec(os::Screen& s);

    static os::Screen& hex(os::Screen& s);

    static os::Screen& boolalpha(os::Screen& s);

    static os::Screen& noboolalpha(os::Screen& s);

    static os::Screen& uppercase(os::Screen& s);

    static os::Screen& nouppercase(os::Screen& s);

    static Manip<char> setfill(char ch);

    static Manip<size_t> setw(size_t width);

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

    os::Screen& operator <<(long long num);

    os::Screen& operator <<(unsigned char num);

    os::Screen& operator <<(unsigned short num);

    os::Screen& operator <<(unsigned int num);

    os::Screen& operator <<(unsigned long num);

    os::Screen& operator <<(unsigned long long num);

    os::Screen& operator <<(const void* ptr);

    os::Screen& operator <<(os::Screen& (*fPtr)(os::Screen&));

    template<typename T>
    os::Screen& operator <<(os::Screen::Manip<T> manip)
    {
        manip.exec(*this);
        return *this;
    }

private:
    static const int SCREEN_WIDTH;
    static const int SCREEN_HEIGHT;
    static const int TAB_SIZE;

    static const uint8_t BOOL_ALPHA;
    static const uint8_t UPPERCASE;

    static void setFill(os::Screen& s, char ch);

    static void setWidth(os::Screen& s, size_t width);

    uint16_t* textMem;
    uint16_t attrib;
    int csrX;
    int csrY;

    int base;
    uint8_t flags;
    size_t width;
    char fill;

    void outputChar(char ch);

    void updateCursor();

    void scroll();

    void rawWrite(char ch);

    void justify(size_t strLen);

    template<typename T>
    void writeSigned(T num);

    template<typename T>
    void writeUnsigned(T num);

    void digitToChar(char& digit);
};

} // namespace os

extern os::Screen screen;

#endif // SCREEN_H_
