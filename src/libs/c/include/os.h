#ifndef _OS_H
#define _OS_H 1

#ifdef __cplusplus
extern "C"
{
#endif

enum EColor
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

void clearTerminal();

int getNumModules();

void getModuleName(int index, char* name);

void setTerminalBackground(enum EColor color);

void setTerminalForeground(enum EColor color);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _OS_H */
