# Makefile for kernel

SRCDIR = .
OBJDIR = obj
LIBSRCDIR = ../../../../libs
LIBDIR = ../../../../../lib
BINDIR = ../../../../../bin
TOOLSDIR = ../../../../../tools

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
	   paging.s.o \
	   screen.cpp.o \
	   shell.cpp.o \
	   system.c.o \
	   system.s.o \
	   timer.cpp.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))
BUILD_OBJ = $(OBJDIR)/crti.s.o $(OBJ) $(OBJDIR)/crtn.s.o
LINK_OBJ = $(OBJDIR)/crti.s.o $(CRTBEGIN_OBJ) $(OBJ) $(CRTEND_OBJ) $(OBJDIR)/crtn.s.o

TARGET = $(BINDIR)/kernel-x86

.PHONY: all
all: init libs $(TARGET)

.PHONY: init
init:
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)

.PHONY: libs
libs:
	cd $(LIBSRCDIR)/c++; make
	cd $(LIBSRCDIR)/c; make

.PHONY: iso
iso: init libs $(TARGET)
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
	cd $(LIBSRCDIR)/c++; make clean
	cd $(LIBSRCDIR)/c; make clean
	rm -f $(OBJDIR)/*.o $(TARGET)
