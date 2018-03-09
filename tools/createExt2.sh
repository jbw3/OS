#!/bin/bash
# create an ext2 image

# THIS SCRIPT DOES NOT WORK YET!

if [[ $# -ne 1 ]]; then
    echo "Error: expected 1 argument"
    exit 1
fi

ARCH_NAME=$1
IMG_NAME=bin/OS-$ARCH_NAME.img

# create new disk image
dd if=/dev/zero of=$IMG_NAME bs=512 count=131072

# create new DOS partition table with bootable entry for filesystem
printf "n\np\n1\n\n\na\n1\nw\n" | fdisk $IMG_NAME

# set up two loops: one for GRUB and the other for the OS's filesystem
sudo losetup /dev/loop0 $IMG_NAME
sudo losetup /dev/loop1 $IMG_NAME -o 1048576

# create ext2 filesystem
sudo mke2fs /dev/loop1

# mount
sudo mount -t ext2 /dev/loop1 /mnt

# install GRUB
sudo grub-install --root-directory=/mnt --no-floppy --modules="normal part_msdos ext2 multiboot biosdev" /dev/loop0

sync

umount /mnt
sudo losetup -d /dev/loop0
sudo losetup -d /dev/loop1
