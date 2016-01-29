# OS


## Requirements

### Compiler

To build the OS kernel you will need a cross-compiler.
GCC 5.3.0 is the compiler currently being used.
Some other compilers or GCC versions will probably work, but note that support for the C11 and C++14 standards is required.
Scripts to build a GCC cross-compiler and the binutils library (needed to build GCC) can be found in the tools directory.
A tutorial (from which these scripts were taken) can be found here:
http://wiki.osdev.org/GCC_Cross-Compiler

### Assembler
This project uses the NASM assembler.


## Building

To build the OS kernel, run `make` in the top-level directory. An ISO image can also be created by running `make install`. Both the kernel binary and ISO image will be placed in the bin directory.
