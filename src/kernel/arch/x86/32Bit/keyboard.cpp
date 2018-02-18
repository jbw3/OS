#include <ctype.h>

#include "keyboard.h"
#include "os.h"
#include "processmgr.h"
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
    KEY_UP, // up arrow
    0, // page up
    '-',
    KEY_LEFT, // left arrow
    0,
    KEY_RIGHT, // right arrow
    '+',
    0, // end
    KEY_DOWN, // down arrow
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

uint8_t Keyboard::scanCodeQueue[Keyboard::SCAN_CODE_QUEUE_SIZE];
unsigned int Keyboard::scanCodeQHead = 0;
unsigned int Keyboard::scanCodeQTail = 0;

uint16_t Keyboard::keyQueue[KEY_QUEUE_SIZE];
unsigned int Keyboard::keyQHead = 0;
unsigned int Keyboard::keyQTail = 0;

void Keyboard::init()
{
    registerIrqHandler(IRQ_KEYBOARD, interruptHandler);
}

void Keyboard::interruptHandler(const registers* /*regs*/)
{
    // read from the keyboard's data buffer
    uint8_t scanCode = inb(0x60);

    // add the scan code to the queue if it is not full
    if ( (scanCodeQTail != scanCodeQHead - 1) && !(scanCodeQHead == 0 && scanCodeQTail == SCAN_CODE_QUEUE_SIZE - 1) )
    {
        scanCodeQueue[scanCodeQTail] = scanCode;
        if (scanCodeQTail >= SCAN_CODE_QUEUE_SIZE - 1)
        {
            scanCodeQTail = 0;
        }
        else
        {
            ++scanCodeQTail;
        }
    }
}

bool Keyboard::getKey(uint16_t& key)
{
    processQueue();

    // if the queue is empty return false
    if (keyQHead == keyQTail)
    {
        return false;
    }

    key = keyQueue[keyQHead];
    if (keyQHead >= KEY_QUEUE_SIZE - 1)
    {
        keyQHead = 0;
    }
    else
    {
        ++keyQHead;
    }

    return true;
}

bool Keyboard::getChar(char& ch)
{
    uint16_t key = 0;

    bool found = false;
    bool isAscii = false;
    bool isPrintable = false;
    do
    {
        found = getKey(key);
        ch = static_cast<char>(key & 0x7F);

        isAscii = (key & 0x7F) == key;
        isPrintable = isprint(ch) || ch == '\t' || ch == '\n';
    } while (found && !(isAscii && isPrintable));

    return found;
}

void Keyboard::processQueue()
{
    while (scanCodeQHead != scanCodeQTail)
    {
        // read scan code from queue
        uint16_t scanCode = scanCodeQueue[scanCodeQHead];
        if (scanCodeQHead >= SCAN_CODE_QUEUE_SIZE - 1)
        {
            scanCodeQHead = 0;
        }
        else
        {
            ++scanCodeQHead;
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

ssize_t Keyboard::read(uint8_t* buff, size_t nbyte)
{
    size_t idx = 0;
    while (idx < nbyte)
    {
        char ch = '\0';
        bool found = getChar(ch);
        while (!found)
        {
            processMgr.yieldCurrentProcess();
            found = getChar(ch);
        }

        reinterpret_cast<char*>(buff)[idx] = ch;
        ++idx;
    }

    return static_cast<ssize_t>(idx);
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
        // check if this is an ASCII char
        if ( (key & 0x7F) == key )
        {
            char ch = static_cast<char>(key);

            // shift the char if
            // 1. either of the shift keys are pressed OR
            // 2. caps lock is active and the char is an alphabet char
            if ( (controlPressed & KEY_SHIFT) != 0 ||
                 ( (controlPressed & KEY_CAPS_LOCK) != 0 && ch >= 'a' && ch <= 'z' ) )
            {
                key = shift(ch);
            }
        }

        addKey(key);
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

void Keyboard::addKey(uint16_t key)
{
    // the key to the queue if it is not full
    if ( (keyQTail != keyQHead - 1) && !(keyQHead == 0 && keyQTail == KEY_QUEUE_SIZE - 1) )
    {
        keyQueue[keyQTail] = key;
        if (keyQTail >= KEY_QUEUE_SIZE - 1)
        {
            keyQTail = 0;
        }
        else
        {
            ++keyQTail;
        }
    }
}

} // namespace os
