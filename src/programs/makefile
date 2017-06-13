# Makefile for test programs

BINDIR = ../../bin/programs
OBJDIR = obj

CC = i686-elf-gcc
CFLAGS = -std=c11 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra

CXX = i686-elf-g++
CXXFLAGS = -std=c++1z -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -fno-exceptions -fno-rtti -Wall -Wextra

AS = nasm
ASFLAGS = -f bin

PROGRAMS = test1 test2 test3 test4
BINARIES = $(patsubst %,$(BINDIR)/%,$(PROGRAMS))

LDFLAGS = -T link.ld -L $(OBJDIR) -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs

# don't use default rules
.SUFFIXES:

.PRECIOUS: $(patsubst %,$(OBJDIR)/%.o,wrapper.s test3.c test4.cpp)

.PHONY: all
all: init $(BINARIES)

.PHONY: init
init:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)

$(BINDIR)/%: %.s
	$(AS) $< $(ASFLAGS) -o $@

$(OBJDIR)/%.s.o: %.s
	$(AS) $< -f elf32 -o $@

$(BINDIR)/%: $(OBJDIR)/%.c.o $(OBJDIR)/wrapper.s.o
	$(CC) $^ -o $@ $(LDFLAGS)

$(BINDIR)/%: $(OBJDIR)/%.cpp.o $(OBJDIR)/wrapper.s.o
	$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJDIR)/%.c.o: %.c
	$(CC) -c $< $(CFLAGS) -o $@

$(OBJDIR)/%.cpp.o: %.cpp
	$(CXX) -c $< $(CXXFLAGS) -o $@

.PHONY: clean
clean:
	rm -f $(BINARIES)
