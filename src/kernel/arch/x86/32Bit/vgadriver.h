#ifndef VGA_DRIVER_H_
#define VGA_DRIVER_H_

#include "stream.h"

class VgaDriver : public Stream
{
public:
    static constexpr unsigned int SCREEN_WIDTH = 80;
    static constexpr unsigned int SCREEN_HEIGHT = 25;
    static constexpr unsigned int TAB_SIZE = 4;

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

    unsigned int getCursorX() const;

    void setCursorX(unsigned int x);

    unsigned int getCursorY() const;

    void setCursorY(unsigned int y);

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
    static constexpr char ESCAPE = '\x1B';
    static constexpr char CSI    = '[';
    static constexpr char CSI_START_PARAMETER_BYTE    = '\x30';
    static constexpr char CSI_END_PARAMETER_BYTE      = '\x3F';
    static constexpr char CSI_START_INTERMEDIATE_BYTE = '\x20';
    static constexpr char CSI_END_INTERMEDIATE_BYTE   = '\x2F';
    static constexpr char CSI_START_FINAL_BYTE        = '\x40';
    static constexpr char CSI_END_FINAL_BYTE          = '\x7E';

    uint16_t* textMem;
    uint16_t attrib;
    unsigned int csrX;
    unsigned int csrY;
    bool inEscSequence;
    char escSequenceChar;
    enum ECsiState
    {
        eParameter,
        eIntermediate,
        eFinal
    } csiState;
    static constexpr size_t MAX_PARAMETER_BYTES_SIZE = 32;
    size_t parameterBytesSize;
    char parameterBytes[MAX_PARAMETER_BYTES_SIZE];
    static constexpr size_t MAX_INTERMEDIATE_BYTES_SIZE = 32;
    size_t intermediateBytesSize;
    char intermediateBytes[MAX_INTERMEDIATE_BYTES_SIZE];
    char finalByte;

    void writeChar(char ch);

    void outputChar(char ch);

    void scroll();

    void updateCursor();

    /**
     * @brief Parse a character in an escape sequence.
     * @param ch the character to parse.
     */
    void parseEscSequence(char ch);

    void parseCsi(char ch);

    void evalCsi();

    bool getNumParam(unsigned int& num, unsigned int def, bool& error, const char*& ptr);
};

#endif // VGA_DRIVER_H_
