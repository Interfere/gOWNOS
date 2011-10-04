# Makefile from JamesM's kernel tutorials
# The C and C++ rules are already setup by default.
# The only one that needs is the assembler 
# as we use nasm instead of GNU as

SOURCES= boot.o main.o common.o monitor.o

CFLAGS= -nostdlib -nostdinc -fno-builtin -fno-stack-protector
LDFLAGS=-Tlink.ld
ASFLAGS=-felf

all: $(SOURCES) link

clean:
	-rm *.o kernel

link:
	ld $(LDFLAGS) -o kernel $(SOURCES)

.s.o:
	nasm $(ASFLAGS) $<
