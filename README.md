# OS


## Requirements

### Compiler

To build the OS kernel you will need a cross-compiler.
GCC 5.3.0 is the compiler currently being used.
Some other compilers or GCC versions will probably work, but note that support for the C11 and C++14 standards is required.
Scripts to build a GCC cross-compiler and the binutils library (needed to build GCC) can be found in the tools directory.
A tutorial (from which these scripts were taken) can be found here:
<http://wiki.osdev.org/GCC_Cross-Compiler>.

### Assembler

This project uses the NASM assembler.


## Building

To build the OS kernel, run `make` in the top-level directory. A GRUB-bootable ISO image can also be created by running `make install`. Both the kernel binary and ISO image will be placed in the bin directory.


## Running

### Bochs

A Bochs configuration file to boot the ISO image can be found in the top-level directory.
The command is `bochs -f bochsrc.txt`.

### QEMU

QEMU can be used to run either the kernel binary or the ISO image. The respective commands are
`qemu-system-i386 -kernel bin/kernel-x86`
and
`qemu-system-i386 -cdrom bin/SandboxOS-x86.iso`.

### Bare Metal

The ISO image can be copied to a USB drive using the following command replacing `sdx` with the USB drive:
`sudo dd if=bin/SandboxOS-x86.iso of=/dev/sdx`.

**IMPORTANT**: The `dd` command will wipe the contents of your USB drive. Also, if you accidentally point it to another drive (say, your hard drive), it may wipe that too!
