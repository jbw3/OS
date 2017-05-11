# Makefile

# config variables
ARCH_PATH = src/kernel/arch
X86_32_PATH = $(ARCH_PATH)/x86/32Bit

.PHONY: release
release:
	cd $(X86_32_PATH); make release

.PHONY: debug
debug:
	cd $(X86_32_PATH); make debug

.PHONY: iso
iso:
	cd $(X86_32_PATH); make iso

.PHONY: clean
clean:
	cd $(X86_32_PATH); make clean

.PHONY: install-gcc32
install-gcc32:
	./tools/build-gcc.py
