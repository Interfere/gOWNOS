// paging.c -- Defines the interface for and structures relating to paging
// From JamesM's kernel development tutorials

#include "paging.h"
#include "kheap.h"
#include "monitor.h"

// The kernel's page directory
page_directory_t *kernel_directory=0;

// The current page directory
page_directory_t *current_directory=0;

// A bitset of freames - used or free
u32int *frames;
u32int nframes;

// Defined in kheap.c
extern u32int placement_address;
extern heap_t *kheap;

// Macros used in the bitset algorithms
#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

// Static function to set a bit int the frames bitset
static void set_frame(u32int frame_addr)
{
	u32int frame = frame_addr/0x1000;
	u32int idx = INDEX_FROM_BIT(frame);
	u32int off = OFFSET_FROM_BIT(frame);
	frames[idx] |= (0x1 << off);
}

static void clear_frame(u32int frame_addr)
{
	u32int frame = frame_addr/0x1000;
	u32int idx = INDEX_FROM_BIT(frame);
	u32int off = OFFSET_FROM_BIT(frame);
	frames[idx] &= ~(0x1 << off);
}

static u32int test_frame(u32int frame_addr)
{
	u32int frame = frame_addr/0x1000;
	u32int idx = INDEX_FROM_BIT(frame);
	u32int off = OFFSET_FROM_BIT(frame);
	return (frames[idx] & (0x1 << off));
}

static u32int first_frame()
{
	u32int i, j;
	for (i = 0; i < INDEX_FROM_BIT(nframes); ++i)
	{
		if (frames[i] != 0xFFFFFFFF) // nothing free, exit early
			for (j = 0; j < 32; ++j)
			{
				u32int toTest = 0x1 << j;
				if(!(frames[i]&toTest))
					return i*4*8+j;
			}
	}
}

// Function to allocate frame
void alloc_frame(page_t *page, int is_kernel, int is_writeable)
{
	if (page->frame != 0)
		return; // Кадр уже выделен для данной страницы
	else
	{
		u32int idx = first_frame(); // index of the first frame
		if(idx == (u32int)-1)
			PANIC("No free frames!");

		set_frame(idx*0x1000); // застолбили кадр
		page->present = 1;
		page->rw = (is_writeable)?1:0;
		page->user = (is_kernel)?0:1;
		page->frame = idx;
	}
}

void free_frame(page_t *page)
{
	u32int frame;
	if(!(frame = page->frame))
		return; // Кадр для данной страницы не выделен
	else
	{
		clear_frame(frame);
		page->frame = 0x0;
	}
}

void initialise_paging()
{
	// Пусть размер нашей памяти - 16 МБ
	// пока что
	u32int mem_end_page = 0x1000000;

	nframes = mem_end_page / 0x1000;
	frames = (u32int*)kmalloc(INDEX_FROM_BIT(nframes));
	memset(frames, 0, INDEX_FROM_BIT(nframes));

	// Создаем каталог страниц
	kernel_directory = (page_directory_t*)kmalloc_a(sizeof(page_directory_t));
	memset(kernel_directory, 0, sizeof(page_directory_t));
	current_directory = kernel_directory;

    /**
      * Map some pages in the kernel heap area.
      * Here we call get_page but not alloc_frame. This causes page_table_t's
      * to be created where necessary. We can't allocate frames yet because they need
      * to be identity mapped first below, and yet we can't increase placement_addres
      * between identity mapping and enabling the heap.
      */
    int i = 0;
    for(i = KHEAP_START; i < KHEAP_START + KHEAP_MIN_SIZE; i += PAGE_SIZE)
        get_page(i, 1, kernel_directory);

    // We should place heap_t structure before we turn paging on
    u32int kheap_addr = kmalloc(sizeof(heap_t));

	/**
	 * Теперь нам необходимо тождественно отобразить
	 * адреса виртуальной памяти на адреса физической памяти,
	 * чтобы мы могли прозрачно обращаться к физической памяти,
	 * как будто страничная адресация не включена.
	 * Обратите внимание что мы специально используем здесь
	 * цикл while, т.к. внутри тела цикла значение переменной
	 * placement_address изменяется при вызове kmalloc().
	 */
    i = 0;
	while (i < placement_address)
	{
		// Код ядра доступен для чтения но не для записи
		// из пространства пользователя
		alloc_frame( get_page(i, 1, kernel_directory),0,0);
		i += 0x1000;
	}

    // Now allocate those pages we mapped earlier
    for(i = KHEAP_START; i < KHEAP_START + KHEAP_MIN_SIZE; i += PAGE_SIZE)
        alloc_frame( get_page(i, 0, kernel_directory), 0, 0 );

    // Before we enable paging, we must register our page fault handler
	register_interrupt_handler(14, page_fault);

    // Enable paging
	switch_page_directory(kernel_directory);

    // Initialise the kernel heap.
    kheap = create_heap(kheap_addr, KHEAP_START, KHEAP_START + KHEAP_MIN_SIZE, 0xCFFFF000, 0, 0);
}

void switch_page_directory(page_directory_t *dir)
{
	current_directory = dir;
	__asm__ volatile ("mov %0, %%cr3"::"r"(&dir->tablesPhysical));
	u32int cr0;
	__asm__ volatile ("mov %%cr0, %0": "=r"(cr0));
	cr0 |= 0x80000000; // ВКЛ
	__asm__ volatile ("mov %0, %%cr0":: "r"(cr0));
}

page_t *get_page(u32int address, int make, page_directory_t *dir)
{
	// Делаем из адреса индекс
	address /= 0x1000;
	// Находим таблицу, содержащую адрес
	u32int table_idx = address / 1024;
	if (dir->tables[table_idx]) // Если таблица уже создана
		return &dir->tables[table_idx]->pages[address%1024];
	else if(make)
	{
		u32int tmp;
		dir->tables[table_idx] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &tmp);
		memset(dir->tables[table_idx], 0, 0x1000);
		dir->tablesPhysical[table_idx] = tmp | 0x7; // PRESENT, RW, US
		return &dir->tables[table_idx]->pages[address%1024];
	}
	else
		return 0;
}

void page_fault(registers_t regs)
{
	// Произошло прерывание page fault
	// Адрес по которому произошло прерывание содержится в регистре CR2
	u32int faulting_address;
	__asm__ volatile ("mov %%cr2, %0" : "=r"(faulting_address));

	// Код ошибки сообщит нам подробности произошедшего
	int present = !(regs.err_code & 0x1);	// Page not present
	int rw = regs.err_code & 0x2;			// Write operation ?
	int us = regs.err_code & 0x4;			// Processor was in user-mode?
	int reserved = regs.err_code & 0x8;		// Overwritten CPU reserved bits
	int id = regs.err_code & 0x10;			// Caused by an instruction

	// Error message
	monitor_write("Page fault! (");
	if (present) monitor_write("present ");
	if (rw) monitor_write("read-only ");
	if (us) monitor_write("user-mode ");
	if (reserved) monitor_write("reserved ");
	monitor_write(") at 0x");
	monitor_write_hex(faulting_address);
	monitor_write("\n");
	PANIC("Page fault");
}

