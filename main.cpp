// main.c

#include <stdint.h>
#include <string.h>

struct multiboot;

int main(struct multiboot* mbootPtr)
{
    uint32_t a1[2] = {0xDEADBEEF, 0xFEEDFACE};
    uint32_t a2[2];

    memcpy(a2, a1, sizeof(uint32_t) * 2);
    memset(a2, 0xAB, sizeof(uint32_t) * 2);

    uint32_t ret = a2[1];
    return ret;
}
