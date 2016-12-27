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
CFLAGS = $(INCLUDES) -std=c11 -ffreestanding -O2 -Wall -Wextra

CXX = x86_64-elf-g++
CXXFLAGS = $(INCLUDES) -std=c++14 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

AS = nasm
ASFLAGS = -f elf64 -I$(SRCDIR)/

LIBS = -L$(LIBDIR) #-lgcc
LD = x86_64-elf-ld
LDFLAGS = -T $(SRCDIR)/link.ld --nmagic -nostdlib $(LIBS)

DEPS =

# CRTBEGIN_OBJ = $(shell $(CXX) $(CXXFLAGS) -print-file-name=crtbegin.o)
# CRTEND_OBJ = $(shell $(CXX) $(CXXFLAGS) -print-file-name=crtend.o)
_OBJ = boot.s.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))
# BUILD_OBJ = $(OBJDIR)/crti.s.o $(OBJ) $(OBJDIR)/crtn.s.o
# LINK_OBJ = $(OBJDIR)/crti.s.o $(CRTBEGIN_OBJ) $(OBJ) $(CRTEND_OBJ) $(OBJDIR)/crtn.s.o
BUILD_OBJ = $(OBJ)
LINK_OBJ = $(OBJ)

TARGET = $(BINDIR)/kernel-x64

.PHONY: all
all: init $(TARGET)

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
	$(TOOLSDIR)/createIso.sh x64

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
