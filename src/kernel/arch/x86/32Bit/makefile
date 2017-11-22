# Makefile for kernel

SRCDIR = .
DEPDIR = .dep
OBJDIR = obj
LIBSRCDIR = ../../../../libs
LIBDIR = ../../../../../lib
BINDIR = ../../../../../bin
TOOLSDIR = ../../../../../tools

# compiler flags for generating dependency makefiles
# -MT:  the name of the target in the dependency file
# -MMD: generate dependency files while compiling; also ignore system headers
# -MP:  add a target for each prerequisite to avoid errors when deleting files
# -MF:  the name of the dependency file
C_DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$<.Td

# assembler flags for generating dependency makefiles
A_DEPFLAGS = -MT $@ -MD $(DEPDIR)/$<.Td -MP

# directory to search for dependencies
VPATH = $(SRCDIR)

INCLUDES = -I$(SRCDIR) -I$(LIBSRCDIR)/c/include -I$(LIBSRCDIR)/c++/include

CC = i686-elf-gcc
CFLAGS = $(INCLUDES) -std=c11 -ffreestanding -Wall -Wextra $(C_DEPFLAGS)

CXX = i686-elf-g++
CXXFLAGS = $(INCLUDES) -std=c++1z -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti $(C_DEPFLAGS)

AS = nasm
ASFLAGS = -f elf32 -I$(SRCDIR)/ $(A_DEPFLAGS)

LIBS = -L$(LIBDIR) -lc -lc++ -lgcc
LDFLAGS = -T $(SRCDIR)/link.ld -ffreestanding -nostdlib $(LIBS)

# After compiling, rename the temporary dependency file to its actual name.
# This is so compilation failures will not result in a corrupted dependency file.
# Also, touch the object file to make sure it is not older than the dependency
# file.
POSTCOMPILE = @mv -f $(DEPDIR)/$<.Td $(DEPDIR)/$<.d && touch $@

release: CFLAGS += -O2
release: CXXFLAGS += -O2
release: CONFIG = release
debug: CFLAGS += -O0 -g
debug: CXXFLAGS += -O0 -g
debug: ASFLAGS += -g
debug: CONFIG = debug

# build list of source files
SRC = $(notdir $(wildcard $(SRCDIR)/*.c $(SRCDIR)/*.cpp $(SRCDIR)/*.s))

# source files without crti.s and crtn.s
NO_CRT_SRC = $(filter-out crt%.s,$(SRC))

CRTBEGIN_OBJ = $(shell $(CXX) $(CXXFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ = $(shell $(CXX) $(CXXFLAGS) -print-file-name=crtend.o)
BUILD_OBJ = $(patsubst %,$(OBJDIR)/%.o,$(SRC))
LINK_OBJ = $(OBJDIR)/crti.s.o $(CRTBEGIN_OBJ) $(patsubst %,$(OBJDIR)/%.o,$(NO_CRT_SRC)) $(CRTEND_OBJ) $(OBJDIR)/crtn.s.o

TARGET = $(BINDIR)/kernel-x86

.PHONY: release
release: init libs $(TARGET)

.PHONY: debug
debug: init libs $(TARGET)

.PHONY: init
init:
	@mkdir -p $(DEPDIR)
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)

.PHONY: libs
libs:
	cd $(LIBSRCDIR)/c++; make $(CONFIG)
	cd $(LIBSRCDIR)/c; make $(CONFIG)

.PHONY: iso
iso:
	$(TOOLSDIR)/createIso.sh x86

.PHONY: run
run: release iso
	qemu-system-i386 -cdrom $(BINDIR)/SandboxOS-x86.iso

.PHONY: debugger
debugger: debug iso
	qemu-system-i386 -s -S -cdrom $(BINDIR)/SandboxOS-x86.iso &
	gdb -x $(TOOLSDIR)/x86.gdb

$(TARGET): $(BUILD_OBJ)
	$(CXX) $(LINK_OBJ) -o $(TARGET) $(LDFLAGS)

$(OBJDIR)/%.c.o: %.c $(DEPDIR)/%.c.d
	$(CC) -c $< $(CFLAGS) -o $@
	$(POSTCOMPILE)

$(OBJDIR)/%.cpp.o: %.cpp $(DEPDIR)/%.cpp.d
	$(CXX) -c $< $(CXXFLAGS) -o $@
	$(POSTCOMPILE)

$(OBJDIR)/%.s.o: %.s $(DEPDIR)/%.s.d
	$(AS) $< $(ASFLAGS) -o $@
	$(POSTCOMPILE)

# rule for dependency files (this is here to keep make happy)
$(DEPDIR)/%.d: ;

# prevent make from deleting dependency files
.PRECIOUS: $(patsubst %,$(DEPDIR)/%.d,$(SRC))

.PHONY: clean
clean:
	cd $(LIBSRCDIR)/c++; make clean
	cd $(LIBSRCDIR)/c; make clean
	rm -f $(TARGET) $(OBJDIR)/*.o $(DEPDIR)/*.d

# include dependency makefiles
include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(SRC)))
