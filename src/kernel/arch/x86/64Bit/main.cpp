/**
 * @brief 64-bit x86 kernel main
 */
extern "C"
void kernelMain()
{
    const int SCREEN_WIDTH = 80;
    const int SCREEN_HEIGHT = 25;
    unsigned short* textMem = reinterpret_cast<unsigned short*>(0xB8000);

    // clear screen
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; ++i)
    {
        textMem[i] = 0x0020;
    }

    while (true);
}
