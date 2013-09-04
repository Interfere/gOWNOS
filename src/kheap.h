// kheap.h -- Interface for kernel heap functions, also provides
//            a placement malloc() for use before the heap is 
//            initialised.
//            Written for JamesM's kernel development tutorials.

#ifndef KHEAP_H
#define KHEAP_H

#include "common.h"
#include "llist.h"

#define KHEAP_START         0xC0000000
#define KHEAP_MIN_SIZE      0x100000

/**
 * @brief The heap struct
 * start_addr   The start of our allocated space
 * end_add      The end of our allocated space. May be expanded up to max_addr.
 * max_addr     The maximum address the heap can be expanded to.
 * supervisor   Should extra pages requested by us be mapped as supervisor-only?
 * readonly     Should extra pages requested by us be mapped as read-only?
 * head         The head of the list.
 */
typedef struct kernel_heap {
    size_t              start_addr;
    size_t              end_addr;
    size_t              max_addr;
    u8int               supervisor;
    u8int               readonly;
    struct list_head    head;
} heap_t;

/**
 * @brief The heap_chunk struct
 * prev_foot    The size of previous block if it is free
 * head         The header itself. Contains size + extra bits.
 * list         prev,next pointers. Set if block is free.
 */
typedef struct heap_chunk {
    size_t              prev_foot;
    size_t              head;
    struct list_head    list;
} hchunk_t;

/**
   Create a new heap.
**/
heap_t *create_heap(u32int placement_addr, u32int start, u32int end,
                    u32int max, u8int supervisor, u8int readonly);

/**
   Allocates a contiguous region of memory 'size' in size.
**/
void *alloc(size_t bytes, heap_t *heap);

/**
   Allocates a contiguous region of memory 'size' in size. it creates that block starting
   on a page boundary.
**/
void *paligned_alloc(size_t bytes, heap_t *heap);

/**
   Releases a block allocated with 'alloc'.
**/
void free(void *p, heap_t *heap);

/**
   Allocate a chunk of memory, sz in size. If align == 1,
   the chunk must be page-aligned. If phys != 0, the physical
   location of the allocated chunk will be stored into phys.

   This is the internal version of kmalloc. More user-friendly
   parameter representations are available in kmalloc, kmalloc_a,
   kmalloc_ap, kmalloc_p.
**/
u32int kmalloc_int(u32int sz, int align, u32int *phys);

/**
   Allocate a chunk of memory, sz in size. The chunk must be
   page aligned.
**/
u32int kmalloc_a(u32int sz);

/**
   Allocate a chunk of memory, sz in size. The physical address
   is returned in phys. Phys MUST be a valid pointer to u32int!
**/
u32int kmalloc_p(u32int sz, u32int *phys);

/**
   Allocate a chunk of memory, sz in size. The physical address 
   is returned in phys. It must be page-aligned.
**/
u32int kmalloc_ap(u32int sz, u32int *phys);

/**
   General allocation function.
**/
u32int kmalloc(u32int sz);

/**
   General freeing function.
**/
void kfree(u32int addr);

#endif // KHEAP_H
