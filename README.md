# OS


## Quick Start

```sh
# clone the repo
git clone git@github.com:jbw3/OS.git
cd OS

# install dependencies
sudo apt install -y nasm xorriso grub-pc-bin
./tools/build-gcc.py

# build OS (as ISO image)
make release iso

# run in QEMU
qemu-system-i386 -cdrom bin/OS-x86.iso
```


## Requirements

### Building the Cross-Compiler

To build the OS kernel you will need a cross-compiler.
GCC 7.2.0 is the compiler currently being used.
Some other compilers or GCC versions will probably work, but note that support for the C11 and C++17 standards is required.
A script to build a GCC cross-compiler can be found in the tools directory: `build-gcc.py`.
A tutorial can also be found at [OSDev](http://wiki.osdev.org/GCC_Cross-Compiler).
Run `build-gcc.py -h` for help and command line options.

### Assembler

This project uses the NASM assembler.
On Ubuntu, it can be installed as follows:
```
sudo apt install nasm
```


## Building

To build the OS kernel, run `make` in the top-level directory. A GRUB-bootable ISO image can also be created by running `make iso`. Both the kernel binary and ISO image will be placed in the bin directory.

Note: The `xorriso` and `grub-pc-bin` packages may be needed to create the ISO image.
On Ubuntu, they can be installed as follows:
```
sudo apt install xorriso
sudo apt install grub-pc-bin
```


## Running

### QEMU

Use the following command to run the OS in QEMU:
```
qemu-system-i386 -cdrom bin/OS-x86.iso
```

The `-serial stdio` option may also be added to redirect a serial port to the
terminal. The OS runs a shell with IO redirected to this serial port
giving you an OS shell in your terminal:
```
qemu-system-i386 -serial stdio -cdrom bin/OS-x86.iso
```

### Bare Metal

The ISO image can be copied to a USB drive using the following command replacing `sdx` with the USB drive:

```
sudo dd if=bin/OS-x86.iso of=/dev/sdx
```

**IMPORTANT**: The `dd` command will wipe the contents of your USB drive. Also, if you accidentally point it to another drive (say, your hard drive), it may wipe that too!


## Shell

The OS has a simple shell for debugging purposes. Type `help` to list the available commands.

![help command](./doc/screenShots/shell-help.png "help command")
