/*
 * Linker script for the kernel
 */

KERNEL_VIRTUAL_BASE = 0xC0000000;
KERNEL_TEXT_OFFSET  = 0x00100000;

ENTRY(start)

SECTIONS
{
    . = (KERNEL_VIRTUAL_BASE + KERNEL_TEXT_OFFSET);

    kernelVirtualStart = .;
    kernelPhysicalStart = . - KERNEL_VIRTUAL_BASE;

    loadStartAddr = . - KERNEL_VIRTUAL_BASE;

    .text ALIGN(4096) : AT(ADDR(.text) - KERNEL_VIRTUAL_BASE)
    {
        *(.text)  /* text sections from all files */
        *(.ctors) /* table of pointers for global C++ constructors */
        *(.dtors) /* table of pointers for global C++ destructors */
    }

    .rodata ALIGN(4096) : AT(ADDR(.rodata) - KERNEL_VIRTUAL_BASE)
    {
        *(.rodata) /* read-only data from all files */
    }

    .data ALIGN(4096) : AT(ADDR(.data) - KERNEL_VIRTUAL_BASE)
    {
        *(.data) /* data sections from all files */
    }

    loadEndAddr = . - KERNEL_VIRTUAL_BASE;

    .bss ALIGN(4096) : AT(ADDR(.bss) - KERNEL_VIRTUAL_BASE)
    {
        *(.bss) /* BSS sections from all files */
    }

    bssEndAddr = . - KERNEL_VIRTUAL_BASE;

    kernelVirtualEnd = .;
    kernelPhysicalEnd = . - KERNEL_VIRTUAL_BASE;
}
