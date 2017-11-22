/*
 * Linker script for the kernel
 */

ENTRY(start)

SECTIONS
{
    kernelVirtualBase = 0xC0000000;
    kernelTextOffset  = 0x00100000;

    . = (kernelVirtualBase + kernelTextOffset);

    kernelVirtualStart = .;
    kernelPhysicalStart = . - kernelVirtualBase;

    loadStartAddr = . - kernelVirtualBase;

    .text ALIGN(4096) : AT(ADDR(.text) - kernelVirtualBase)
    {
        /* The multiboot header must be at the start of the file. */
        *(.multiboot_header)

        /* The start section is the first section of kernel code that is
           executed. The page this section is in is identity mapped in the page
           table, so it can execute before the kernel jumps to the higher half.
         */
        *(.start)

        /* Text sections from all files. */
        *(.text)

        /* Table of pointers for global C++ constructors. */
        *(.ctors)

        /* Table of pointers for global C++ destructors. */
        *(.dtors)
    }

    .rodata ALIGN(4096) : AT(ADDR(.rodata) - kernelVirtualBase)
    {
        *(.rodata) /* read-only data from all files */
    }

    .data ALIGN(4096) : AT(ADDR(.data) - kernelVirtualBase)
    {
        *(.data) /* data sections from all files */
    }

    loadEndAddr = . - kernelVirtualBase;

    .bss ALIGN(4096) : AT(ADDR(.bss) - kernelVirtualBase)
    {
        *(.bss) /* BSS sections from all files */
    }

    bssEndAddr = . - kernelVirtualBase;

    kernelVirtualEnd = .;
    kernelPhysicalEnd = . - kernelVirtualBase;
}
