#!/bin/bash

# create an ISO image
mkdir -p isodir/boot/grub
cp kernel isodir/boot/kernel
cp grub.cfg isodir/boot/grub/grub.cfg
grub-mkrescue -o SandboxOS.iso isodir
