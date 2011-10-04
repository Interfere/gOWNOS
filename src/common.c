// common.c -- Defines some global functions

#include "common.h"

// write a byte out to the specified port
void outb(u16int port, u8int value)
{
	__asm__ volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

u8int inb(u16int port)
{
	u8int ret;
	__asm__ volatile ("inb %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

u16int inw(u16int port)
{
	u16int ret;
	__asm__ volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

// Copy len bytes from src to dest.
void memcpy(void *dest, const void *src, u32int len)
{
    const u8int *sp = (const u8int *)src;
    u8int *dp = (u8int *)dest;
    while(len--) 
		*dp++ = *sp++;
}

// Write len copies of val into dest.
void memset(void *dest, u8int val, u32int len)
{
    u8int *temp = (u8int *)dest;
    while (len--) 
		*temp++ = val;
}

// Returns an integral value indicating the relationship between the strings:
// A zero value indicates that both strings are equal.
// A value greater than zero indicates that the first character that does not 
// match has a greater value in str1 than in str2; And a value less than zero indicates the opposite.
int strcmp(const char *str1, const char *str2)
{
	register signed char __res;
	while(1) {
		if((__res = *str1 - *str2++) != 0 || *str1++)
			break;
	}
	return __res;
}

// Copy the NULL-terminated string src into dest, and
// return dest.
char *strcpy(char *dest, const char *src)
{
	char* tmp = dest;

	while((*dest++ = *src++) != '\0');

	return tmp;
}

// Concatenate the NULL-terminated string src onto
// the end of dest, and return dest.
char *strcat(char *dest, const char *src)
{
	char* tmp = dest;

	while(*dest)
		dest++;
	while((*dest++ = *src++) != '\0');

	return tmp;
}
