// main.c -- Defines the C-code kernel entry point, calls initialisation routines.


void kmain(int magic, struct multiboot *mboot_ptr)
{
	// Check for multiboot magic
	if(magic != 0x2BADB002)
	{
		// error. Bootloader not multiboot-compliant
		return;
	}
	// All our initialisation calls will go in here
}

