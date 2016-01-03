#!/bin/bash
# create an ISO image

if [[ $# -ne 1 ]]; then
    echo "Error: expected 1 argument"
    exit 1
fi

ARCH_NAME=$1

mkdir -p bin/isodir-$ARCH_NAME/boot/grub
cp bin/kernel-$ARCH_NAME bin/isodir-$ARCH_NAME/boot/kernel
cp grub.cfg bin/isodir-$ARCH_NAME/boot/grub/grub.cfg
grub-mkrescue bin/isodir-$ARCH_NAME -o bin/SandboxOS-$ARCH_NAME.iso
