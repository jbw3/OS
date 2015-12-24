# Makefile

INCLUDES = -I.

CC = gcc
CFLAGS = $(INCLUDES) -std=c99 -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector

LDFLAGS = -Tlink.ld -melf_i386

ASFLAGS = -f elf32

ODIR = obj
_OBJ = boot.o main.o string.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

TARGET = kernel

.PHONY: all
all: init $(TARGET)

.PHONY: init
init:
	mkdir -p $(ODIR)

$(TARGET): $(OBJ)
	ld $(LDFLAGS) $(OBJ) -o $(TARGET)

$(ODIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(ODIR)/%.o: %.s
	nasm $(ASFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o $(TARGET)
