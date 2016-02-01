# Makefile

SRCDIR = src
OBJDIR = obj
BINDIR = bin

INCLUDES = -I$(SRCDIR)
VPATH = $(SRCDIR)

CC = i686-elf-gcc
CFLAGS = $(INCLUDES) -std=c11 -ffreestanding -O2 -Wall -Wextra

CXX = i686-elf-g++
CXXFLAGS = $(INCLUDES) -std=c++14 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

AS = nasm
ASFLAGS = -f elf32

LDFLAGS = -T $(SRCDIR)/link.ld -ffreestanding -O2 -nostdlib -lgcc

DEPS = debug.h gdt.h idt.h irq.h isr.h keyboard.h paging.h screen.h shell.h stddef.h stdint.h stdio.h stdlib.h string.h stringutils.h system.h timer.h

CRTBEGIN_OBJ = $(shell $(CXX) $(CXXFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ = $(shell $(CXX) $(CXXFLAGS) -print-file-name=crtend.o)
_OBJ = boot.o debug.o gdt.o idt.o interrupt.o irq.o isr.o keyboard.o main.o paging.o paging_asm.o screen.o shell.o stdlib.o stdio.o string.o stringutils.o system.o system_asm.o timer.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))
BUILD_OBJ = $(OBJDIR)/crti.o $(OBJ) $(OBJDIR)/crtn.o
LINK_OBJ = $(OBJDIR)/crti.o $(CRTBEGIN_OBJ) $(OBJ) $(CRTEND_OBJ) $(OBJDIR)/crtn.o

TARGET = $(BINDIR)/kernel-x86

TOOLSDIR = ./tools

.PHONY: all
all: init $(TARGET)

.PHONY: init
init:
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)

.PHONY: install
install: init $(TARGET)
	$(TOOLSDIR)/createIso.sh x86

$(TARGET): $(BUILD_OBJ)
	$(CXX) $(LINK_OBJ) -o $(TARGET) $(LDFLAGS)

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o $(TARGET)
