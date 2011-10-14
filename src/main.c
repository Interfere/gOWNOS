// main.c -- Defines the C-code kernel entry point, calls initialisation routines.

#include "monitor.h"
#include "descriptor_tables.h"
#include "paging.h"

void kmain(int magic, struct multiboot *mboot_ptr)
{
	if(magic != 0x2BADB002)
	{
		// error. Bootloader not multiboot-compliant
		return;
	}
	monitor_clear();
	// All our initialisation calls will go in here
	// Setting up GDT and IDT
	init_descriptor_tables();
	// Allow IRQs
	__asm__ volatile ("sti");

	initialise_paging();
	monitor_write("Hello, paging world!\n");

	u32int *ptr = (u32int*)0xA0000000;
	u32int do_page_fault = *ptr;
}

