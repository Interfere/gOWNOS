// descriptor_tables.h 

#include "common.h"

// Инициализирующая функция
extern void init_descriptor_tables();

// Эта структура содержит значения для одной записи GDT
struct gdt_entry_struct {
	u16int limit_low;	// Младшие 16 бит смещения
	u16int base_low;	// Младшие 16 бит базы
	u8int  base_middle;	// Следующие восемь бит базы
	u8int  access;		// Флаг определяет уровень доступа
	u8int  granularity;
	u8int  base_high;	// старшие 8 бит базы
} __attribute__((packed));

typedef struct gdt_entry_struct gdt_entry_t;

struct gdt_ptr_struct {
	u16int limit;	// старшие 16 бит смещения селектора
	u32int base;	// адрес первой структуры gdt_entry_t
} __attribute__((packed));

typedef struct gdt_ptr_struct gdt_ptr_t;

// Структура описывает запись в IDT
struct idt_entry_struct {
	u16int base_lo;		// Первые 16 бит адреса начала обработчика прерывания
	u16int sel;			// Селектор сегмента ядра
	u8int  always0;		// Всегда должно быть равно нулю
	u8int  flags;		// Флаги. RTFM.
	u16int base_hi;		// Старшие 16 бит адреса начала обработчика прерывания
}__attribute__((packed));

typedef struct idt_entry_struct idt_entry_t;

// Структура описывает указатель на массив обработчиков прерываний
// в формате пригодном для загрузки в специальный регистр
struct idt_ptr_struct {
	u16int limit;
	u32int base;
}__attribute__((packed));

typedef struct idt_ptr_struct idt_ptr_t;

// Следующие директивы позволят нам обращаться к адресам обработчиков
// описанных в ASM файле
extern void isr0 ();
extern void isr1 ();
extern void isr2 ();
extern void isr3 ();
extern void isr4 ();
extern void isr5 ();
extern void isr6 ();
extern void isr7 ();
extern void isr8 ();
extern void isr9 ();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();


