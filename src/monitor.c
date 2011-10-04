// monitor.c -- Defines the interface for monitor

#include "monitor.h"

static u16int* video_memory = (u16int*)0xB8000;

static u8int cursor_x = 0;
static u8int cursor_y = 0;

// Обновляет позицию курсора
static void move_cursor()
{
	// Ширина экрана 80 символов...
	u16int cursorLocation = cursor_y*80 + cursor_x;
	outb(0x3D4, 14);					// Мы собираемся послать старший байт координаты курсора...
	outb(0x3D5, cursorLocation >> 8);	// ... и посылаем его.
	outb(0x3D4, 15);					// Мы собираемся послать младший байт координаты курсора...
	outb(0x3D5, cursorLocation);		// ... и посылаем его.
}

// Функцию для прокрутки экрана
static void scroll()
{
	// Создаем пробельный символ с установками цвета по умолчанию
	u8int attributeByte = (0 /*black*/ << 4) | (15 /*white*/ & 0x0F);
	u16int blank = 0x20 /*space*/ | (attributeByte << 8);

	// 25 строка - последняя. Это значит, что мы должны прокрутить экран.
	if(cursor_y >= 25)
	{
		// Копируем i строку в i-1
		int i;
		for (i = 0;i < 24*80; ++i)
		{
			video_memory[i] = video_memory[i+80];
		}

		// Последняя строка должна быть пустой
		for(i = 24*80; i < 25*80;++i)
		{
			video_memory[i] = blank;
		}
		// Курсор должне быть на последней строке
		cursor_y = 24;
	}
}

// Выводим символ на экран
void monitor_put(char c)
{
	// Цвет символа - белый(15), цвет фона - черный(0)
	u8int backColor = 0;
	u8int foreColor = 15;

	u8int attributeByte = (backColor << 4) | (foreColor & 0x0F);
	
	u16int attribute = attributeByte << 8;
	u16int *location;

	if(c == 0x08 && cursor_x) // <BackSpace>
		--cursor_x;
	else if(c == 0x09) // <TAB>
		cursor_x = (cursor_x+8) & ~(8-1);
	else if(c == '\r')
		cursor_x = 0;
	else if(c == '\n')
	{
		cursor_x = 0;
		++cursor_y;
	}
	else if(c >= ' ')
	{
		location = video_memory + (cursor_y*80 + cursor_x);
		*location = c | attribute;
		++cursor_x;
	}

	if(cursor_x >= 80)
	{
		cursor_x = 0;
		++cursor_y;
	}

	scroll();
	move_cursor();
}

void monitor_clear()
{
	// Создаем пробельный символ с установками цвета по умолчанию
	u8int attributeByte = (0 /*black*/ << 4) | (15 /*white*/ & 0x0F);
	u16int blank = 0x20 /*space*/ | (attributeByte << 8);

	int i;
	for(i=0;i<80*25;++i)
	{
		video_memory[i] = blank;
	}

	cursor_x = 0;
	cursor_y = 0;
	move_cursor();
}

// Выводит нуль-терминированную строку на экран
void monitor_write(char* c)
{
	int i = 0;
	while(c[i])
		monitor_put(c[i++]);
}

void monitor_write_hex(u32int n)
{
	s32int tmp;
	monitor_write("0x");

	char noZeroes = 1;

	int i;
	for(i = 28; i > 0; i -= 4)
	{
		tmp = (n>>i) & 0xF;
		if(tmp == 0 && noZeroes != 0)
		{
			continue;
		}

		if(tmp >= 0xA)
		{
			noZeroes = 0;
			monitor_put(tmp - 0xA+'a');
		}
		else
		{
			noZeroes = 0;
			monitor_put(tmp+'0');
		}
	}

	tmp = n & 0xF;
	if( tmp >= 0xA )
		monitor_put(tmp-0xA+'a');
	else
		monitor_put(tmp+'0');
}

void monitor_write_dec(u32int n)
{
	if(n==0)
	{
		monitor_put('0');
		return;
	}

	s32int acc = n;
	char c[32];
	int i = 0;
	while(acc > 0)
	{
		c[i] = '0' + acc%10;
		acc /= 10;
		++i;
	}
	c[i] = 0;

	char c2[32];
	c2[i--] = 0;
	int j = 0;
	while(i >= 0)
		c2[i--] = c[j++];
	
	monitor_write(c2);
}

