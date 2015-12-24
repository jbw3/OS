// main.c

#include <stdint.h>
#include <string.h>

#include "screen.h"

struct multiboot;

int main(struct multiboot* mbootPtr)
{
    os::Screen screen;

    screen.clear();

    screen.setBackgroundColor(os::Screen::EColor::eBlack);
    screen.setForegroundColor(os::Screen::EColor::eLightGreen);

    screen.write('A');
    screen.write('B');
    screen.write('C');
    screen.write('\n');
    screen.write('D');

    while (true);

    return 0;
}
