#ifndef PAGING_H_
#define PAGING_H_

#include "common.h"
#include "isr.h"

typedef struct page
{
	u32int present	: 1;	// Страница представлена в памяти
	u32int rw		: 1;	// Если установлен - то read-only
	u32int user		: 1;	// Если сброшен - то уровень ядра
	u32int accessed	: 1;	// Был ли доступ к странице
	u32int dirty	: 1;	// Была ли запись в страницу
	u32int unused	: 7;	// Зарезервированные и неиспользуемые биты
	u32int frame	: 20;	// Адрес кадра
} page_t;

typedef struct page_table
{
	page_t pages[1024];
} page_table_t;

typedef struct page_directory
{
	/**
	 * Массив указателей на таблицы страниц
	 */
	page_table_t *tables[1024];
	/**
	 * Массив указателей на адреса в физической памяти,
	 * в уоторых располагаются таблицы страниц.
	 * Используется для загрузки значений в CR3
	 */
	u32int tablesPhysical[1024];
	/**
	 * Физический адрес tablesPhysical. Используется,
	 * если куча ядра уже выделена, а каталог страниц
	 * находится не в ней
	 */
} page_directory_t;

/**
 * Настраивает окружение и включает страничную адресацию
 */
extern void initialise_paging();

/**
 * Загружает адрес каталога страниц в регистр CR3
 */
extern void switch_page_directory(page_directory_t *new);

/**
 * Возвращает указатель на запрашиваемую страницу
 * Если make=1 и таблица страниц не существует, то
 * создает таблицу.
 */
extern page_t *get_page(u32int address, int make, page_directory_t *dir);

/**
 * Обработчик Page fault
 */
extern void page_fault(registers_t regs);

#endif

