// common.h -- Defines typedefs and some global functions

#ifndef COMMON_H_
#define COMMON_H_

// Некоторые определения, чтобы стандартизировать типы
// Эти типы определены для платформы x86
#ifdef __i386__
typedef unsigned int	u32int;
typedef          int	s32int;
typedef unsigned short	u16int;
typedef          short	s16int;
typedef unsigned char	u8int;
typedef          char	s8int;
#else
#error "Types for non-x86 not implemented."
#endif

extern void outb(u16int port, u8int value);

extern u8int inb(u16int port);

extern u16int inw(u16int port);

extern void memcpy(void *dest, const void *src, u32int len);

extern void memset(void *dest, u8int val, u32int len);

extern int strcmp(const char *str1, const char *str2);

extern char *strcpy(char *dest, const char *src);

extern char *strcat(char *dest, const char *src);

#endif

