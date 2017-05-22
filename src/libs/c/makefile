# Makefile for C lib

SRCDIR = src
DEPDIR = .dep
OBJDIR = obj
BINDIR = ../../../lib

# compiler flags for generating dependency makefiles
# -MT:  the name of the target in the dependency file
# -MMD: generate dependency files while compiling; also ignore system headers
# -MP:  add a target for each prerequisite to avoid errors when deleting files
# -MF:  the name of the dependency file
C_DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$(notdir $<).Td

# directory to search for dependencies
VPATH = $(SRCDIR) include

INCLUDES = -I$(SRCDIR) -Iinclude -I../c++/include

CC = i686-elf-gcc
CFLAGS = $(INCLUDES) -std=c11 -ffreestanding -Wall -Wextra $(C_DEPFLAGS)

CXX = i686-elf-g++
CXXFLAGS = $(INCLUDES) -std=c++1z -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti $(C_DEPFLAGS)

# After compiling, rename the temporary dependency file to its actual name.
# This is so compilation failures will not result in a corrupted dependency file.
# Also, touch the object file to make sure it is not older than the dependency
# file.
POSTCOMPILE = @mv -f $(DEPDIR)/$(notdir $<).Td $(DEPDIR)/$(notdir $<).d && touch $@

release: CFLAGS += -O2
release: CXXFLAGS += -O2
debug: CFLAGS += -O0 -g
debug: CXXFLAGS += -O0 -g

# build list of source files
SRC = $(notdir $(wildcard $(SRCDIR)/*.c $(SRCDIR)/*.cpp $(SRCDIR)/*.s))

OBJ = $(patsubst %,$(OBJDIR)/%.o,$(SRC))

TARGET = $(BINDIR)/libc.a

.PHONY: release
release: init $(TARGET)

.PHONY: debug
debug: init $(TARGET)

.PHONY: init
init:
	@mkdir -p $(DEPDIR)
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)

$(TARGET): $(OBJ)
	ar rcs $(TARGET) $(OBJ)

$(OBJDIR)/%.c.o: %.c $(DEPDIR)/%.c.d
	$(CC) $(CFLAGS) -c $< -o $@
	$(POSTCOMPILE)

$(OBJDIR)/%.cpp.o: %.cpp $(DEPDIR)/%.cpp.d
	$(CXX) $(CXXFLAGS) -c $< -o $@
	$(POSTCOMPILE)

# rule for dependency files (this is here to keep make happy)
$(DEPDIR)/%.d: ;

# prevent make from deleting dependency files
.PRECIOUS: $(patsubst %,$(DEPDIR)/%.d,$(SRC))

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJDIR)/*.o $(DEPDIR)/*.d

# include dependency makefiles
include $(wildcard $(patsubst %,$(DEPDIR)/%.d,$(SRC)))
