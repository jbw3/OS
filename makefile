# Makefile

# config variables
THIRD_PARTY_INSTALL_FOLDER = ~/opt
DOWNLOAD_FOLDER = ~/Downloads
BINUTILS_VERSION = 2.27
GCC_VERSION = 6.2.0
GCC_FOLDER = $(THIRD_PARTY_INSTALL_FOLDER)/gcc-$(GCC_VERSION)-cross

BINUTILS_FILENAME = binutils-$(BINUTILS_VERSION).tar.gz
BINUTILS_URL_PREFIX = http://ftp.gnu.org/gnu/binutils
BINUTILS_DOWNLOAD = $(DOWNLOAD_FOLDER)/$(BINUTILS_FILENAME)
GCC_FILENAME = gcc-$(GCC_VERSION).tar.bz2
GCC_URL_PREFIX = http://mirrors.concertpass.com/gcc/releases/gcc-$(GCC_VERSION)
GCC_DOWNLOAD = $(DOWNLOAD_FOLDER)/$(GCC_FILENAME)

ARCH_PATH = src/kernel/arch
X86_32_PATH = $(ARCH_PATH)/x86/32Bit

.PHONY: all
all:
	cd $(X86_32_PATH); make

.PHONY: iso
iso:
	cd $(X86_32_PATH); make iso

.PHONY: clean
clean:
	cd $(X86_32_PATH); make clean

.PHONY: install-gcc32
install-gcc32:
	# download gcc & binutils
	curl $(BINUTILS_URL_PREFIX)/$(BINUTILS_FILENAME) > $(BINUTILS_DOWNLOAD)
	curl $(GCC_URL_PREFIX)/$(GCC_FILENAME) > $(DOWNLOAD_FOLDER)/$(GCC_FILENAME)

	./tools/build-gcc.py $(BINUTILS_DOWNLOAD) $(GCC_DOWNLOAD) -o $(GCC_FOLDER)

	echo Add the gcc install directory listed above to your path!