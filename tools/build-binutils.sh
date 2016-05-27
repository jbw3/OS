#!/bin/bash

export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"
export BIN_UTILS_DIR=binutils-2.26

mkdir build-binutils
cd build-binutils
../$BIN_UTILS_DIR/configure --target=$TARGET --prefix="$PREFIX" --with-sysroot --disable-nls --disable-werror
make
make install

