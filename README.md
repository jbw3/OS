# OS


## Requirements

### Compiler

To build the OS kernel you will need a cross-compiler.
GCC 5.4.0 is the compiler currently being used.
Some other compilers or GCC versions will probably work, but note that support for the C11 and C++14 standards is required.
Scripts to build a GCC cross-compiler and the binutils library (needed to build GCC) can be found in the tools directory.
A tutorial (from which these scripts were taken) can be found at [OSDev](http://wiki.osdev.org/GCC_Cross-Compiler).
Following are the steps to build GCC:

1. Download [Binutils](https://www.gnu.org/software/binutils/)
2. Configure variables in build-binutils.sh
3. Build Binutils (run build-binutils.sh)
4. Download [GCC](https://gcc.gnu.org/)
5. Configure variables in build-gcc.sh
6. Build GCC (run build-gcc.sh)

### Assembler

This project uses the NASM assembler.


## Building

To build the OS kernel, run `make` in the top-level directory. A GRUB-bootable ISO image can also be created by running `make install`. Both the kernel binary and ISO image will be placed in the bin directory.


## Running

### Bochs

A Bochs configuration file to boot the ISO image can be found in the top-level directory.
The command is `bochs -f bochsrc-linux.txt`.

### QEMU

QEMU can be used to run either the kernel binary or the ISO image. The respective commands are
`qemu-system-i386 -kernel bin/kernel-x86`
and
`qemu-system-i386 -cdrom bin/SandboxOS-x86.iso`.

### Bare Metal

The ISO image can be copied to a USB drive using the following command replacing `sdx` with the USB drive:

```
sudo dd if=bin/SandboxOS-x86.iso of=/dev/sdx
```

**IMPORTANT**: The `dd` command will wipe the contents of your USB drive. Also, if you accidentally point it to another drive (say, your hard drive), it may wipe that too!


## Shell

The OS has a simple shell for debugging purposes. Type `help` to list the available commands. Type `help <cmd>` to get help on a specific command.

![help command](./docs/screenShots/cmd_help.png "help command")
