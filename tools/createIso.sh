#!/bin/bash
# create an ISO image

if [[ $# -ne 1 ]]; then
    echo "Error: expected 1 argument"
    exit 1
fi

NAME=$0
FULL_NAME=$(readlink -f $NAME)
DIR_NAME=$(dirname ${FULL_NAME})

TOP_DIR=${DIR_NAME}/..
BIN_DIR=${TOP_DIR}/bin
ARCH_NAME=$1
ISO_DIR=${BIN_DIR}/isodir-${ARCH_NAME}

# create directories
mkdir -p ${ISO_DIR}/boot/grub
mkdir -p ${ISO_DIR}/modules

# copy kernel
cp ${BIN_DIR}/kernel-${ARCH_NAME} ${ISO_DIR}/boot/kernel

# copy GRUB config file
cp ${TOP_DIR}/grub.cfg ${ISO_DIR}/boot/grub/grub.cfg

# copy modules
cp ${TOP_DIR}/bin/test1 ${ISO_DIR}/modules
cp ${TOP_DIR}/bin/test2 ${ISO_DIR}/modules

# make grub image
grub-mkrescue /usr/lib/grub/i386-pc ${ISO_DIR} -o ${BIN_DIR}/SandboxOS-${ARCH_NAME}.iso
