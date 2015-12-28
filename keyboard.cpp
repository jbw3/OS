#include "keyboard.h"
#include "screen.h"
#include "system.h"

#define CONTROL_KEYS_START 0x0100

#define KEY_LEFT_SHIFT  0x0100
#define KEY_RIGHT_SHIFT 0x0200
#define KEY_SHIFT       (KEY_LEFT_SHIFT | KEY_RIGHT_SHIFT)
#define KEY_ALT         0x0400

namespace os
{

const uint16_t Keyboard::LAYOUT_US[128] =
{
    0,
    27, // escape
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',
    '=',
    '\b', // backspace
    '\t', // tab
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    '\n', // enter
    0, // control
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',
    '\'',
    '`',
    KEY_LEFT_SHIFT, // left shift
    '\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    KEY_RIGHT_SHIFT, // right shift
    '*',
    0, // alt
    ' ',
    0, // caps lock
    0, // F1
    0, // F2
    0, // F3
    0, // F4
    0, // F5
    0, // F6
    0, // F7
    0, // F8
    0, // F9
    0, // F10
    0, // num lock
    0, // scroll lock
    0, // home
    0, // up arrow
    0, // page up
    '-',
    0, // left arrow
};

uint16_t Keyboard::controlPressed = 0;

void Keyboard::init()
{
    installIrqHandler(IRQ_KEYBOARD, interruptHandler);
}

void Keyboard::interruptHandler(const registers* regs)
{
    // read from the keyboard's data buffer
    uint8_t scanCode = inb(0x60);

    // if bit 7 is set, the key was just released
    if ((scanCode & 0x80) != 0)
    {
        scanCode &= 0x7F;
        keyRelease(LAYOUT_US[scanCode]);
    }
    else
    {
        keyPress(LAYOUT_US[scanCode]);
    }
}

void Keyboard::keyRelease(uint16_t key)
{
    if (key >= CONTROL_KEYS_START)
    {
        controlPressed &= ~key;
    }
}

void Keyboard::keyPress(uint16_t key)
{
    // if the keypress was not mapped to a key, do nothing
    if (key == 0)
    {
        return;
    }

    if (key >= CONTROL_KEYS_START)
    {
        controlPressed |= key;
    }
    else
    {
        char ch = static_cast<char>(key);
        // check if shift is pressed
        if ((controlPressed & KEY_SHIFT) != 0)
        {
            ch = shift(ch);
        }
        screen.write(ch);
    }
}

char Keyboard::shift(char ch)
{
    if (ch >= 'a' && ch <= 'z')
    {
        // convert to upper case
        return ( ch & 0xDF );
    }
    else
    {
        switch (ch)
        {
        case '`':
            return '~';
        case '1':
            return '!';
        case '2':
            return '@';
        case '3':
            return '#';
        case '4':
            return '$';
        case '5':
            return '%';
        case '6':
            return '^';
        case '7':
            return '&';
        case '8':
            return '*';
        case '9':
            return '(';
        case '0':
            return ')';
        case '-':
            return '_';
        case '=':
            return '+';
        case '[':
            return '{';
        case ']':
            return '}';
        case '\\':
            return '|';
        case ';':
            return ':';
        case '\'':
            return '"';
        case ',':
            return '<';
        case '.':
            return '>';
        case '/':
            return '?';
        }
    }

    // if there was no mapping, return the original char
    return ch;
}

} // namespace os
