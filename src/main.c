
#include "monitor.h"

void kmain(int magic, struct multiboot *mboot_ptr)
{
	// Check for multiboot magic
	if(magic != 0x2BADB002)
	{
		// error. Bootloader not multiboot-compliant
		return;
	}
	// All our initialisation calls will go in here
	monitor_clear();
	char* licns = 
"                    GNU GENERAL PUBLIC LICENSE\n"
"                       Version 2, June 1991\n"
"\n"
" Copyright (C) 1989, 1991 Free Software Foundation, Inc.,\n"
" 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA\n"
" Everyone is permitted to copy and distribute verbatim copies\n"
" of this license document, but changing it is not allowed.\n"
"\n"
"                            Preamble\n"
"\n"
"  The licenses for most software are designed to take away your\n"
"freedom to share and change it.  By contrast, the GNU General Public\n"
"License is intended to guarantee your freedom to share and change free\n"
"software--to make sure the software is free for all its users.  This\n"
"General Public License applies to most of the Free Software\n"
"Foundation's software and to any other program whose authors commit to\n"
"using it.  (Some other Free Software Foundation software is covered by\n"
"the GNU Lesser General Public License instead.)  You can apply it to\n"
"your programs, too.\n"
"\n"
"  When we speak of free software, we are referring to freedom, not\n"
"price.  Our General Public Licenses are designed to make sure that you\nhave the freedom to distribute copies of free software (and charge for\n"
"this service if you wish), that you receive source code or can get it\n"
"if you want it, that you can change the software or use pieces of it\n"
"in new free programs; and that you know you can do these things.\n"
"\n"
"  To protect your rights, we need to make restrictions that forbid"
"anyone to deny you these rights or to ask you to surrender the rights."
"These restrictions translate to certain responsibilities for you if you"
"distribute copies of the software, or if you modify it."
"\n"
"  For example, if you distribute copies of such a program, whether"
"gratis or for a fee, you must give the recipients all the rights that"
"you have.  You must make sure that they, too, receive or can get the"
"source code.  And you must show them these terms so they know their"
"rights."

"  We protect your rights with two steps: (1) copyright the software, and\n"
"(2) offer you this license which gives you legal permission to copy,\n"
"distribute and/or modify the software.\n"
"\n"
"  Also, for each author's protection and ours, we want to make certain\n"
"that everyone understands that there is no warranty for this free\n"
"software.  If the software is modified by someone else and passed on, we\n"
"want its recipients to know that what they have is not the original, so\n"
"that any problems introduced by others will not reflect on the original\n"
"authors' reputations.\n"
"\n"
"  Finally, any free program is threatened constantly by software\n"
"patents.  We wish to avoid the danger that redistributors of a free\n"
"program will individually obtain patent licenses, in effect making the\n"
"program proprietary.  To prevent this, we have made it clear that any\n"
"patent must be licensed for everyone's free use or not licensed at all.\n"
	;
	monitor_write(licns);
}

