# Makefile to build all programs

PROG_DIRS = $(wildcard */)

CMD=

clean: CMD += clean

.PHONY: all
all: $(PROG_DIRS)

.PHONY: clean
clean: $(PROG_DIRS)

.PHONY: $(PROG_DIRS)
$(PROG_DIRS):
	make -C $@ $(CMD)
