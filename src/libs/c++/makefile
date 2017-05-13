# Makefile for C++ lib

SRCDIR = src
OBJDIR = obj
BINDIR = ../../../lib

# directory to search for dependencies
VPATH = $(SRCDIR) include

INCLUDES = -I$(SRCDIR) -Iinclude -I../c/include

CXX = i686-elf-g++
CXXFLAGS = $(INCLUDES) -std=c++1z -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti

release: CXXFLAGS += -O2
debug: CXXFLAGS += -O0 -g

DEPS = cmath

_OBJ = cmath.cpp.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

TARGET = $(BINDIR)/libc++.a

# don't use default rules
.SUFFIXES:

.PHONY: release
release: init $(TARGET)

.PHONY: debug
debug: init $(TARGET)

.PHONY: init
init:
	mkdir -p $(OBJDIR)
	mkdir -p $(BINDIR)

$(TARGET): $(OBJ)
	ar rcs $(TARGET) $(OBJ)

$(OBJDIR)/%.cpp.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJDIR)/*.o $(TARGET)
