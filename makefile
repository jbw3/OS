# Makefile

INCLUDES = -I.

CC = i686-elf-gcc
CFLAGS = $(INCLUDES) -std=c11 -ffreestanding -O2 -Wall -Wextra

CXX = i686-elf-g++
CXXFLAGS = $(INCLUDES) -std=c++14 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

AS = nasm
ASFLAGS = -f elf32

LDFLAGS = -T link.ld -ffreestanding -O2 -nostdlib -lgcc

DEPS = gdt.h idt.h irq.h isr.h keyboard.h screen.h stddef.h stdint.h stdlib.h string.h system.h timer.h

ODIR = obj
_OBJ = boot.o gdt.o idt.o interrupt.o irq.o isr.o keyboard.o main.o screen.o stdlib.o string.o system.o systemasm.o timer.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

BINDIR = bin
TARGET = $(BINDIR)/kernel-x86

TOOLSDIR = ./tools

.PHONY: all
all: init $(TARGET) install

.PHONY: init
init:
	mkdir -p $(ODIR)
	mkdir -p $(BINDIR)

.PHONY: install
install:
	$(TOOLSDIR)/createIso.sh x86

$(TARGET): $(OBJ)
	$(CXX) $(LDFLAGS) $(OBJ) -o $(TARGET)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(ODIR)/%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o $(TARGET)
