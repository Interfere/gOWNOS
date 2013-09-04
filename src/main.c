// main.c -- Defines the C-code kernel entry point, calls initialisation routines.

#include "monitor.h"
#include "descriptor_tables.h"
#include "paging.h"
#include "kheap.h"

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

    u32int a = kmalloc(8);
	initialise_paging();

    u32int b = kmalloc(8);
    u32int c = kmalloc(8);

    monitor_write("a: ");
    monitor_write_hex(a);

    monitor_write(", b: ");
    monitor_write_hex(b);

    monitor_write("\nc: ");
    monitor_write_hex(c);

    kfree(c);
    kfree(b);

    u32int d = kmalloc(12);
    monitor_write(", d: ");
    monitor_write_hex(d);
}

