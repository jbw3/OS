#include "keyboard.h"
#include "screen.h"
#include "system.h"

#define CONTROL_KEYS_START 0x0100

#define KEY_CONTROL     0x0100
#define KEY_LEFT_SHIFT  0x0200
#define KEY_RIGHT_SHIFT 0x0400
#define KEY_SHIFT       (KEY_LEFT_SHIFT | KEY_RIGHT_SHIFT)
#define KEY_ALT         0x0800
#define KEY_CAPS_LOCK   0x1000
#define KEY_NUM_LOCK    0x2000
#define KEY_SCROLL_LOCK 0x4000
#define KEY_LOCKS       (KEY_CAPS_LOCK | KEY_NUM_LOCK | KEY_SCROLL_LOCK)

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
    KEY_CONTROL, // control
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
    KEY_ALT, // alt
    ' ',
    KEY_CAPS_LOCK, // caps lock
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
    KEY_NUM_LOCK, // num lock
    KEY_SCROLL_LOCK, // scroll lock
    0, // home
    0, // up arrow
    0, // page up
    '-',
    0, // left arrow
    0,
    0, // right arrow
    '+',
    0, // end
    0, // down arrow
    0, // page down
    0, // insert
    0, // delete
    0,
    0,
    0,
    0, // F11
    0, // F12
};

uint16_t Keyboard::controlPressed = 0;

uint8_t Keyboard::queue[Keyboard::QUEUE_SIZE];
unsigned int Keyboard::qHead = 0;
unsigned int Keyboard::qTail = 0;

void Keyboard::init()
{
    installIrqHandler(IRQ_KEYBOARD, interruptHandler);
}

void Keyboard::interruptHandler(const registers* regs)
{
    // read from the keyboard's data buffer
    uint8_t scanCode = inb(0x60);

    // add the scan code to the queue if it is not full
    if ( (qTail != qHead - 1) && !(qHead == 0 && qTail == QUEUE_SIZE - 1) )
    {
        queue[qTail] = scanCode;
        if (qTail >= QUEUE_SIZE - 1)
        {
            qTail = 0;
        }
        else
        {
            ++qTail;
        }
    }
}

void Keyboard::processQueue()
{
    while (qHead != qTail)
    {
        // read scan code from queue
        uint16_t scanCode = queue[qHead];
        if (qHead >= QUEUE_SIZE - 1)
        {
            qHead = 0;
        }
        else
        {
            ++qHead;
        }

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
}

void Keyboard::keyRelease(uint16_t key)
{
    if (key >= CONTROL_KEYS_START)
    {
        // don't update caps lock, num lock, or scroll lock on a key release
        if ( (key & KEY_LOCKS) == 0)
        {
            controlPressed &= ~key;
        }
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
        // if the key is caps lock, num lock, or scroll lock, toggle the state
        if ( (key & KEY_LOCKS) != 0 )
        {
            controlPressed ^= key;
            updateLights();
        }
        else // set the key as pressed
        {
            controlPressed |= key;
        }
    }
    else
    {
        char ch = static_cast<char>(key);

        // check if this is a printable char
        if ( (ch >= ' ' && ch <= '~') || ch == '\b' || ch == '\t' || ch == '\n' || ch == '\r' )
        {
            // shift the char if
            // 1. either of the shift keys are pressed OR
            // 2. caps lock is active and the char is an alphabet char
            if ( (controlPressed & KEY_SHIFT) != 0 ||
                 ( (controlPressed & KEY_CAPS_LOCK) != 0 && ch >= 'a' && ch <= 'z' ) )
            {
                ch = shift(ch);
            }
            screen.write(ch);
        }
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

void Keyboard::updateLights()
{
    uint8_t lights = 0;
    if (controlPressed & KEY_SCROLL_LOCK)
    {
        lights |= 0x1;
    }
    if (controlPressed & KEY_NUM_LOCK)
    {
        lights |= 0x2;
    }
    if (controlPressed & KEY_CAPS_LOCK)
    {
        lights |= 0x4;
    }

    // wait until the keyboard controller is ready
    while ( (inb(0x64) & 0x2) != 0 );

    // send command byte
    outb(0x60, 0xED);

    // send lights status
    outb(0x60, lights);
}

} // namespace os
