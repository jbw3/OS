# Makefile for C lib

SRCDIR = src
OBJDIR = obj
BINDIR = ../../../lib

# directory to search for dependencies
VPATH = $(SRCDIR) include

INCLUDES = -I$(SRCDIR) -Iinclude

CC = i686-elf-gcc
CFLAGS = $(INCLUDES) -std=c11 -ffreestanding -O2 -Wall -Wextra

DEPS = ctype.h stddef.h stdint.h stdio.h stdlib.h string.h stringutils.h

_OBJ = ctype.o stdlib.o stdio.o string.o stringutils.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

TARGET = $(BINDIR)/libc.a

.PHONY: all
all: init $(TARGET)

.PHONY: init
init:
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)

$(TARGET): $(OBJ)
	ar rcs $(TARGET) $(OBJ)

$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o $(TARGET)
