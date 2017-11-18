# Command-Line Explanation
# --------------------------
# -M q35 sets the chipset (which includes an Intel ICH-9 controller)
# -drive attaches my .img file to a logical drive
# -device ide-drive attaches a SATA device to my logical drive,
# and the bus=ide.0 argument attaches the SATA device to port 0
# on the ICH9 (which supports AHCI access to the 6 SATA ports)

# good info: https://bugzilla.redhat.com/show_bug.cgi?id=1368300
qemu-system-i386 -kernel bin/kernel-x86 -monitor stdio -M q35 \
-drive file=osDrive.img,if=none,id=osDrive \
-device ide-drive,drive=osDrive,bus=ide.0

# formerly, I used this:
# ----------------------
#qemu-system-i386 -kernel bin/kernel-x86 -monitor stdio -M q35 \
#-hda osDrive.img

# or try this...
# -----------------
# -drive file=osDrive.img,if=none,id=myDrive \
# -device ich9-ahci,id=ahci \
# -device ide-drive,drive=myDrive,bus=ahci.0
