// main.c -- Defines the C-code kernel entry point, calls initialisation routines.

#include "descriptor_tables.h"
#include "timer.h"

void kmain(int magic, struct multiboot *mboot_ptr)
{
	if(magic != 0x2BADB002)
	{
		// error. Bootloader not multiboot-compliant
		return;
	}
	// All our initialisation calls will go in here
	// Setting up GDT and IDT
	init_descriptor_tables();
	// Allow IRQs
	__asm__ volatile ("sti");
	// Initialize timer
	init_timer(50);
}

