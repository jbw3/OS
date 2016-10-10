# Makefile

ARCH_PATH = src/kernel/arch
X86_32_PATH = $(ARCH_PATH)/x86/32Bit

.PHONY: all
all:
	cd $(X86_32_PATH); make

.PHONY: install
install:
	cd $(X86_32_PATH); make install

.PHONY: clean
clean:
	cd $(X86_32_PATH); make clean
