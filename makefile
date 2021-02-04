# Makefile

# config variables
PROG_PATH = src/programs
ARCH_PATH = src/kernel/arch
X86_32_PATH = $(ARCH_PATH)/x86/32Bit

# TODO: add a hierarchy level (cd kernel; make)
# TODO: shouldn't we have ARCH_PATH = $(X86_32_PATH) if we want to
# build x86 and then we generically cd $(ARC_PATH); make?

.PHONY: release
release:
	cd $(X86_32_PATH); make release
	cd $(PROG_PATH); make

.PHONY: debug
debug:
	cd $(X86_32_PATH); make debug
	cd $(PROG_PATH); make

.PHONY: iso
iso:
	cd $(X86_32_PATH); make iso

.PHONY: clean
clean:
	cd $(X86_32_PATH); make clean
	cd $(PROG_PATH); make clean

.PHONY: install-gcc
install-gcc:
	./tools/build-gcc.py -t i686-elf x86_64-elf
