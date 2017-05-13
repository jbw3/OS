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

CC = x86_64-elf-gcc
CFLAGS = $(INCLUDES) -std=c11 -ffreestanding -Wall -Wextra

CXX = x86_64-elf-g++
CXXFLAGS = $(INCLUDES) -std=c++1z -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti

AS = nasm
ASFLAGS = -f elf64 -I$(SRCDIR)/

LIBS = -L$(LIBDIR) #-lgcc
LD = x86_64-elf-ld
LDFLAGS = -T $(SRCDIR)/link.ld --nmagic -nostdlib $(LIBS)

release: CFLAGS += -O2
release: CXXFLAGS += -O2
release: CONFIG = release
debug: CFLAGS += -O0 -g
debug: CXXFLAGS += -O0 -g
debug: ASFLAGS += -g
debug: CONFIG = debug

DEPS =

# CRTBEGIN_OBJ = $(shell $(CXX) $(CXXFLAGS) -print-file-name=crtbegin.o)
# CRTEND_OBJ = $(shell $(CXX) $(CXXFLAGS) -print-file-name=crtend.o)
_OBJ = boot.s.o \
	   main.cpp.o \
	   start64.s.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))
# BUILD_OBJ = $(OBJDIR)/crti.s.o $(OBJ) $(OBJDIR)/crtn.s.o
# LINK_OBJ = $(OBJDIR)/crti.s.o $(CRTBEGIN_OBJ) $(OBJ) $(CRTEND_OBJ) $(OBJDIR)/crtn.s.o
BUILD_OBJ = $(OBJ)
LINK_OBJ = $(OBJ)

TARGET = $(BINDIR)/kernel-x64

.PHONY: release
release: init $(TARGET)

.PHONY: debug
debug: init $(TARGET)

.PHONY: init
init:
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)

.PHONY: libs
libs:
	cd $(LIBSRCDIR)/c++; make $(CONFIG)
	cd $(LIBSRCDIR)/c; make $(CONFIG)

.PHONY: iso
iso: release
	$(TOOLSDIR)/createIso.sh x64

.PHONY: run
run: release
	qemu-system-x86_64 -kernel $(TARGET)

.PHONY: debugger
debugger: debug
	qemu-system-x86_64 -s -S -kernel $(TARGET) &
	gdb -x $(TOOLSDIR)/x64.gdb

$(TARGET): $(BUILD_OBJ)
	$(LD) $(LINK_OBJ) -o $(TARGET) $(LDFLAGS)

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
