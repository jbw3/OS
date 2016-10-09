# Makefile for C lib

SRCDIR = src
OBJDIR = obj
BINDIR = ../../../lib

# directory to search for dependencies
VPATH = $(SRCDIR) include

INCLUDES = -I$(SRCDIR) -Iinclude -I../c++/include

CC = i686-elf-gcc
CFLAGS = $(INCLUDES) -std=c11 -ffreestanding -O2 -Wall -Wextra

CXX = i686-elf-g++
CXXFLAGS = $(INCLUDES) -std=c++14 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

DEPS = ctype.h math.h stddef.h stdint.h stdio.h stdlib.h string.h stringutils.h

_OBJ = ctype.c.o math.cpp.o stdio.c.o stdlib.c.o string.c.o stringutils.c.o
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

$(OBJDIR)/%.c.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.cpp.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o $(TARGET)
