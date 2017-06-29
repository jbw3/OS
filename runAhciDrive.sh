qemu-system-i386 -kernel bin/kernel-x86 -monitor stdio -M q35

#-drive file=osDrive.img,if=none,id=myDrive \
# I don't think I need the ich9-ahci device with -M q35 option b/c it
# ------------
# appears to be built in...
#-device ich9-ahci,id=ahci -device ide-drive,drive=myDrive,bus=ahci.0
