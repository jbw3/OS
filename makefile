# Makefile for kernel

SRCDIR = src
OBJDIR = obj
LIBDIR = lib
BINDIR = bin

# directory to search for dependencies
VPATH = $(SRCDIR)

INCLUDES = -I$(SRCDIR) -I$(SRCDIR)/libs/c/include

CC = i686-elf-gcc
CFLAGS = $(INCLUDES) -std=c11 -ffreestanding -O2 -Wall -Wextra

CXX = i686-elf-g++
CXXFLAGS = $(INCLUDES) -std=c++14 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

AS = nasm
ASFLAGS = -f elf32 -I$(SRCDIR)/

LIBS = -L$(LIBDIR) -lc -lgcc
LDFLAGS = -T $(SRCDIR)/link.ld -ffreestanding -O2 -nostdlib $(LIBS)

DEPS = debug.h gdt.h idt.h irq.h isr.h keyboard.h paging.h screen.h shell.h system.h timer.h

CRTBEGIN_OBJ = $(shell $(CXX) $(CXXFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ = $(shell $(CXX) $(CXXFLAGS) -print-file-name=crtend.o)
_OBJ = boot.o debug.o gdt.o idt.o interrupt.o irq.o isr.o keyboard.o main.o paging.o paging_asm.o screen.o shell.o system.o system_asm.o timer.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))
BUILD_OBJ = $(OBJDIR)/crti.o $(OBJ) $(OBJDIR)/crtn.o
LINK_OBJ = $(OBJDIR)/crti.o $(CRTBEGIN_OBJ) $(OBJ) $(CRTEND_OBJ) $(OBJDIR)/crtn.o

TARGET = $(BINDIR)/kernel-x86

TOOLSDIR = ./tools

.PHONY: all
all: init libs $(TARGET)

.PHONY: init
init:
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)

.PHONY: libs
libs:
	cd src/libs/c; make

.PHONY: install
install: init libs $(TARGET)
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
	cd src/libs/c; make clean
	rm -f $(OBJDIR)/*.o $(TARGET)
