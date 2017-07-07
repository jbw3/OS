# This file is intended to be included in program makefiles.
# This expects the following to be defined:
# NAME - The name of the program
# SRC  - The list of source files to build the program

# TODO: get path where makefile resides
MAKEFILE_DIR = ..

OBJDIR = obj
LIBSRCDIR = $(MAKEFILE_DIR)/../libs
LIBDIR = $(MAKEFILE_DIR)/../../lib
BINDIR = $(MAKEFILE_DIR)/../../bin/programs

SRC += wrapper.s
BUILD_OBJ = $(patsubst %,$(OBJDIR)/%.o,$(SRC))

TARGET = $(BINDIR)/$(NAME)

C_INCLUDES = -I$(LIBSRCDIR)/c/include
CXX_INCLUDES = -I$(LIBSRCDIR)/c/include

CC = i686-elf-gcc
CFLAGS = $(C_INCLUDES) -std=c11 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra

CXX = i686-elf-g++
CXXFLAGS = $(CXX_INCLUDES) -std=c++1z -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -fno-exceptions -fno-rtti -Wall -Wextra

AS = nasm
ASFLAGS = -f bin

LIBS = -L$(OBJDIR) -L$(LIBDIR) -lc -lgcc
LDFLAGS = -T $(MAKEFILE_DIR)/link.ld $(LIBS) -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs

# don't use default rules
.SUFFIXES:

.PHONY: all
all: init $(TARGET)

.PHONY: init
init:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)

$(TARGET): $(BUILD_OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJDIR)/wrapper.s.o: $(MAKEFILE_DIR)/wrapper.s
	$(AS) $< -f elf32 -o $@

$(OBJDIR)/%.s.o: %.s
	$(AS) $< -f elf32 -o $@

$(OBJDIR)/%.c.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

$(OBJDIR)/%.cpp.o: %.cpp
	$(CXX) -c $< $(CXXFLAGS) -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJDIR)/*.o
