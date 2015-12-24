# Makefile

SOURCES=boot.o main.o

CFLAGS=-m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector
LDFLAGS=-Tlink.ld -melf_i386
ASFLAGS=-f elf32

all: $(SOURCES) link

link:
	ld $(LDFLAGS) -o kernel $(SOURCES)

main.o: main.c
	gcc $(CFLAGS) -c main.c

boot.o:
	nasm $(ASFLAGS) boot.s

clean:
	rm -f *.o kernel
