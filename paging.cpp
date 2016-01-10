#include "paging.h"
#include "screen.h"

void initPaging()
{
    installIrqHandler(IRQ_PAGE_FAULT, pageFault);
    initPageDir();
    initPageTable(0);
    // enablePaging();
}

extern "C"
void pageFault(const registers* /*regs*/)
{
    os::Screen::EColor bgColor = screen.getBackgroundColor();
    os::Screen::EColor fgColor = screen.getForegroundColor();

    screen.setBackgroundColor(os::Screen::EColor::eRed);
    screen.setForegroundColor(os::Screen::EColor::eWhite);

    screen << "Page fault!";

    screen.setBackgroundColor(bgColor);
    screen.setForegroundColor(fgColor);

    // hang
    while (true);
}
