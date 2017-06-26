/*
 * Linker script for C/C++ programs
 */

/* flat binary format */
OUTPUT_FORMAT("binary")

SECTIONS
{
    /* start at address 0 */
    . = 0;

    .text ALIGN(4):
    {
        wrapper.s.o(.text) /* include wrapper.s.o text section first */
        *(.text)           /* include all other text sections */
        *(.ctors) /* table of pointers for global C++ constructors */
        *(.dtors) /* table of pointers for global C++ destructors */
    }

    .data ALIGN(4):
    {
        *(.data)
    }

    .rodata ALIGN(4):
    {
        *(.rodata)
    }

    .bss ALIGN(4):
    {
        *(.bss)
    }
}
