# Makefile for JamesM's kernel tutorials
# The C and C++ rules are already setup by default.
# The only one that needs is the assembler 
# as we use nasm instead of GNU as

SOURCES= boot.o main.o monitor.o common.o descriptor_tables.o isr.o interrupts.o descriptors.o timer.o kheap.o paging.o

CFLAGS= -g -nostdlib -nostdinc -fno-builtin -fno-stack-protector
LDFLAGS=-Tlink.ld
ASFLAGS=-felf

all: $(SOURCES) link

clean:
	-rm *.o kernel

link:
	ld $(LDFLAGS) -o kernel $(SOURCES)

.s.o:
	nasm $(ASFLAGS) $<
