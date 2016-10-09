# Makefile for kernel

SRCDIR = src/kernel/arch/x86/32Bit
OBJDIR = obj
LIBSRCDIR = src/libs
LIBDIR = lib
BINDIR = bin

# directory to search for dependencies
VPATH = $(SRCDIR)

INCLUDES = -I$(SRCDIR) -I$(LIBSRCDIR)/c/include -I$(LIBSRCDIR)/c++/include

CC = i686-elf-gcc
CFLAGS = $(INCLUDES) -std=c11 -ffreestanding -O2 -Wall -Wextra

CXX = i686-elf-g++
CXXFLAGS = $(INCLUDES) -std=c++14 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

AS = nasm
ASFLAGS = -f elf32 -I$(SRCDIR)/

LIBS = -L$(LIBDIR) -lc -lc++ -lgcc
LDFLAGS = -T $(SRCDIR)/link.ld -ffreestanding -O2 -nostdlib $(LIBS)

DEPS = debug.h gdt.h idt.h irq.h isr.h keyboard.h pageframemgr.h paging.h screen.h shell.h system.h timer.h

CRTBEGIN_OBJ = $(shell $(CXX) $(CXXFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ = $(shell $(CXX) $(CXXFLAGS) -print-file-name=crtend.o)
_OBJ = boot.s.o \
	   debug.cpp.o \
	   gdt.c.o \
	   idt.c.o \
	   interrupt.s.o \
	   irq.cpp.o \
	   isr.cpp.o \
	   keyboard.cpp.o \
	   main.cpp.o \
	   pageframemgr.cpp.o \
	   paging.cpp.o \
	   paging_asm.s.o \
	   screen.cpp.o \
	   shell.cpp.o \
	   system.c.o \
	   system_asm.s.o \
	   timer.cpp.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))
BUILD_OBJ = $(OBJDIR)/crti.s.o $(OBJ) $(OBJDIR)/crtn.s.o
LINK_OBJ = $(OBJDIR)/crti.s.o $(CRTBEGIN_OBJ) $(OBJ) $(CRTEND_OBJ) $(OBJDIR)/crtn.s.o

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
	cd src/libs/c++; make
	cd src/libs/c; make

.PHONY: install
install: init libs $(TARGET)
	$(TOOLSDIR)/createIso.sh x86

$(TARGET): $(BUILD_OBJ)
	$(CXX) $(LINK_OBJ) -o $(TARGET) $(LDFLAGS)

$(OBJDIR)/%.c.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.cpp.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.s.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

.PHONY: clean
clean:
	cd src/libs/c++; make clean
	cd src/libs/c; make clean
	rm -f $(OBJDIR)/*.o $(TARGET)
