qemu-system-i386 -kernel bin/kernel-x86 -monitor stdio -M q35 \
-hda osDrive.img

# or try this...
# -----------------
# -drive file=osDrive.img,if=none,id=myDrive \
# -device ich9-ahci,id=ahci \
# -device ide-drive,drive=myDrive,bus=ahci.0
