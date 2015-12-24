# Makefile

INCLUDES = -I.

CC = gcc
CFLAGS = $(INCLUDES) -std=c99 -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector

CXX = g++
CXXFLAGS = $(INCLUDES) -std=c++11 -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -Wno-main

ASM = nasm
ASFLAGS = -f elf32

LDFLAGS = -Tlink.ld -melf_i386

ODIR = obj
_OBJ = boot.o main.o screen.o string.o
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

$(ODIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ODIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(ODIR)/%.o: %.s
	$(ASM) $(ASFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o $(TARGET)
