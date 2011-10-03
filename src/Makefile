# Makefile for JamesM's kernel tutorials
# The C and C++ rules are already setup by default.
# The only one that needs is the assembler 
# as we use nasm instead of GNU as

OBJS= boot.o

CFLAGS=
LDFLAGS=-Tlink.ld
ASFLAGS=-felf

all: $(OBJS) link

clean:
	-rm *.o kernel

link:
	ld $(LDFLAGS) -o kernel $(OBJS)

.s.o:
	nasm $(ASFLAGS) $<
