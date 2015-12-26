# Makefile

INCLUDES = -I.

CC = gcc
CFLAGS = $(INCLUDES) -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector

CXX = g++
CXXFLAGS = $(INCLUDES) -std=c++1y -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector

ASM = nasm
ASFLAGS = -f elf32

LDFLAGS = -Tlink.ld -melf_i386

DEPS = screen.h stddef.h stdint.h stdlib.h string.h

ODIR = obj
_OBJ = boot.o main.o screen.o stdlib.o string.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

TARGET = kernel

.PHONY: all
all: init $(TARGET) install

.PHONY: init
init:
	mkdir -p $(ODIR)

.PHONY: install
install:
	./updateImage.sh

$(TARGET): $(OBJ)
	ld $(LDFLAGS) $(OBJ) -o $(TARGET)

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

$(ODIR)/%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(ODIR)/%.o: %.s
	$(ASM) $(ASFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o $(TARGET)
