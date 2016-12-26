/*
 * Linker script for the kernel
 */

ENTRY(start)

SECTIONS
{
    kernelVirtualBase = 0x00000000;/*0xC0000000;*/
    kernelTextOffset  = 0x00100000;

    . = (kernelVirtualBase + kernelTextOffset);

    kernelVirtualStart = .;
    kernelPhysicalStart = . - kernelVirtualBase;

    loadStartAddr = . - kernelVirtualBase;

    .text ALIGN(4096) : AT(ADDR(.text) - kernelVirtualBase)
    {
        *(.text)  /* text sections from all files */
        *(.ctors) /* table of pointers for global C++ constructors */
        *(.dtors) /* table of pointers for global C++ destructors */
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
