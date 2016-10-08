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

mkdir -p ${BIN_DIR}/isodir-${ARCH_NAME}/boot/grub
cp ${BIN_DIR}/kernel-${ARCH_NAME} ${BIN_DIR}/isodir-${ARCH_NAME}/boot/kernel
cp ${TOP_DIR}/grub.cfg ${BIN_DIR}/isodir-${ARCH_NAME}/boot/grub/grub.cfg
grub-mkrescue /usr/lib/grub/i386-pc ${BIN_DIR}/isodir-${ARCH_NAME} -o ${BIN_DIR}/SandboxOS-${ARCH_NAME}.iso
