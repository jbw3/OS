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
    }

    .data ALIGN(4):
    {
        *(.data)
    }

    .rodata ALIGN(4):
    {
        *(.rodata)
    }
}
