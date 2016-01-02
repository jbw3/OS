#!/bin/bash

# create an ISO image
mkdir -p bin/isodir/boot/grub
cp bin/kernel-x86 bin/isodir/boot/kernel
cp grub.cfg bin/isodir/boot/grub/grub.cfg
grub-mkrescue -o bin/SandboxOS.iso bin/isodir
