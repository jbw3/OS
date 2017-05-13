# Makefile

# config variables
ARCH_PATH = src/kernel/arch
X86_32_PATH = $(ARCH_PATH)/x86/32Bit
X86_64_PATH = $(ARCH_PATH)/x86/64Bit

.PHONY: release
release:
	cd $(X86_32_PATH); make release
	cd $(X86_64_PATH); make

.PHONY: debug
debug:
	cd $(X86_32_PATH); make debug
	cd $(X86_64_PATH); make

.PHONY: iso
iso:
	cd $(X86_32_PATH); make iso
	cd $(X86_64_PATH); make iso

.PHONY: clean
clean:
	cd $(X86_32_PATH); make clean
	cd $(X86_64_PATH); make clean

.PHONY: install-gcc32
install-gcc32:
	./tools/build-gcc.py
