// kheap.c -- Kernel heap functions, also provides
//            a placement malloc() for use before the heap is 
//            initialised.
//            Written for JamesM's kernel development tutorials.

#include "kheap.h"
#include "paging.h"


#define SIZE_T_SIZE         (sizeof(size_t))
#define TWO_SIZE_T_SIZES    (SIZE_T_SIZE<<1)
#define SIZE_T_ZERO         ((size_t)0)
#define SIZE_T_ONE          ((size_t)1)
#define SIZE_T_TWO          ((size_t)2)
#define SIZE_T_FOUR         ((size_t)4)

#define PINUSE_BIT          (SIZE_T_ONE)
#define CINUSE_BIT          (SIZE_T_TWO)
#define RESERV_BIT          (SIZE_T_FOUR)
#define INUSE_BITS          (PINUSE_BIT|CINUSE_BIT)
#define FLAG_BITS           (INUSE_BITS|RESERV_BIT)

#define HCHUNK_SIZE         (sizeof(hchunk_t))
#define KMALLOC_ALIGNMENT   ((size_t)(2 * sizeof(void *)))

#define KMALLOC_ALIGN_MASK  (KMALLOC_ALIGNMENT - SIZE_T_ONE)
#define MIN_CHUNK_SIZE      ((HCHUNK_SIZE + KMALLOC_ALIGN_MASK) & ~KMALLOC_ALIGN_MASK)
#define CHUNK_OVERHEAD      (SIZE_T_SIZE)

#define MIN_REQUEST         (MIN_CHUNK_SIZE - CHUNK_OVERHEAD - SIZE_T_ONE)

#define chunksize(ptr)      ((ptr)->head & ~(FLAG_BITS))

#define get_head(heap)      (&(heap)->head)

#define chunk2mem(ptr)      ((void*)((char*)(ptr) + TWO_SIZE_T_SIZES))
#define mem2chunk(ptr)      ((hchunk_t*)((char*)(ptr) - TWO_SIZE_T_SIZES))

#define pinuse(p)           ((p)->head & PINUSE_BIT)
#define cinuse(p)           ((p)->head & CINUSE_BIT)
#define ok_address(p, h)    (((size_t)(p) >= (h)->start_addr) && ((size_t)(p) < (h)->end_addr))
#define ok_inuse(p)         (((p)->head & INUSE_BITS) != PINUSE_BIT)

#define pad_request(req)    \
    (((req) + CHUNK_OVERHEAD + KMALLOC_ALIGN_MASK) & ~KMALLOC_ALIGN_MASK)

#define request2size(req)   (((req) < MIN_REQUEST) ? MIN_CHUNK_SIZE : pad_request(req))

#define chunk_plus_offset(p, s)     ((hchunk_t*)((char*)(p) + (s)))
#define chunk_minus_offset(p, s)    ((hchunk_t*)((char*)(p) - (s)))

#define set_pinuse(p)       ((p)->head |= PINUSE_BIT)
#define clear_pinuse(p)     ((p)->head &= ~PINUSE_BIT)

#define set_inuse(p,s) \
    ((p)->head = (((p)->head & PINUSE_BIT)|CINUSE_BIT|(s)), \
    (chunk_plus_offset(p, s))->head |= PINUSE_BIT)

#define set_size_and_pinuse(p, s) \
    ((p)->head = ((s)|PINUSE_BIT))

#define set_foot(p, s)      (chunk_plus_offset(p, s)->prev_foot = (s))

// end is defined in the linker script.
extern u32int end;
u32int placement_address = (u32int)&end;
extern page_directory_t *kernel_directory;
heap_t *kheap = 0;

u32int kmalloc_int(u32int sz, int align, u32int *phys)
{
    if(kheap)
    {
        void *addr;
        if(align)
        {
            addr = paligned_alloc(sz, kheap);
        }
        else
        {
            addr = alloc(sz, kheap);
        }

        if(phys != 0)
        {
            page_t *page = get_page((u32int)addr, 0, kernel_directory);
            *phys = page->frame*PAGE_SIZE + (((u32int)addr)& -PAGE_SIZE);
        }

        return (u32int)addr;
    }
    else
    {
        // This will eventually call malloc() on the kernel heap.
        // For now, though, we just assign memory at placement_address
        // and increment it by sz. Even when we've coded our kernel
        // heap, this will be useful for use before the heap is initialised.
        if (align == 1 && (placement_address & 0xFFFFF000) )
        {
            // Align the placement address;
            placement_address &= 0xFFFFF000;
            placement_address += 0x1000;
        }
        if (phys)
        {
            *phys = placement_address;
        }
        u32int tmp = placement_address;
        placement_address += sz;
        return tmp;
    }
}

u32int kmalloc_a(u32int sz)
{
    return kmalloc_int(sz, 1, 0);
}

u32int kmalloc_p(u32int sz, u32int *phys)
{
    return kmalloc_int(sz, 0, phys);
}

u32int kmalloc_ap(u32int sz, u32int *phys)
{
    return kmalloc_int(sz, 1, phys);
}

u32int kmalloc(u32int sz)
{
    return kmalloc_int(sz, 0, 0);
}

void kfree(u32int addr)
{
    if(kheap)
    {
        free((void *)addr, kheap);
    }
}

static hchunk_t *find_smallest_chunk(size_t bytes, heap_t* heap)
{
    hchunk_t * tmp = 0;
    struct list_head* iter;
    list_for_each(iter, get_head(heap)) {
        tmp = list_entry(iter, hchunk_t, list);
        if(chunksize(tmp) >= bytes)
        {
            break;
        }
    }

    if (iter == get_head(heap))
    {
        return 0;
    }

    return tmp;
}

static void insert_chunk(hchunk_t *chunk, heap_t *heap)
{
    if(list_empty(get_head(heap)))
    {
        list_add_tail(&(chunk->list), get_head(heap));
    }
    else
    {
        struct list_head* iter;
        size_t csize = chunksize(chunk);
        list_for_each(iter, get_head(heap))
        {
            hchunk_t *tmp = list_entry(iter, hchunk_t, list);
            if(chunksize(tmp) >= csize)
            {
                break;
            }
        }

        list_add_tail(&(chunk->list), iter);
    }
}

#define remove_chunk(chunk)     __list_del_entry(&((chunk)->list))

heap_t *create_heap(u32int placement_addr, u32int start,
                    u32int end, u32int max, u8int supervisor, u8int readonly)
{
    heap_t *heap = (heap_t *)placement_addr;

    // All our assumptions are made on start_addr and end_addr are page-aligned
    ASSERT(start % PAGE_SIZE == 0);
    ASSERT(end % PAGE_SIZE == 0);

    heap->start_addr = start;
    heap->end_addr = end;
    heap->max_addr = max;
    heap->supervisor = supervisor;
    heap->readonly = readonly;
    INIT_LIST_HEAD(get_head(heap));

    // Create first hole
    size_t hole_size = end - start;
    hchunk_t* hole = (hchunk_t*)start;
    hole->prev_foot = 0;
    set_size_and_pinuse(hole, hole_size);

    insert_chunk(hole, heap);

    return heap;
}

static void expand(size_t new_size, heap_t *heap)
{
    ASSERT(new_size > heap->end_addr - heap->start_addr);

    if(new_size % PAGE_SIZE)
    {
        new_size &= -PAGE_SIZE;
        new_size += PAGE_SIZE;
    }

    ASSERT(heap->start_addr + new_size <= heap->max_addr);

    size_t old_size = heap->end_addr - heap->start_addr;
    size_t i = old_size;
    while(i < new_size)
    {
        alloc_frame( get_page(heap->start_addr + i, 1, kernel_directory),
                     heap->supervisor, !heap->readonly);
        i += PAGE_SIZE;
    }

    heap->end_addr = heap->start_addr + new_size;
}

static size_t contract(size_t new_size, heap_t *heap)
{
    ASSERT(new_size < heap->end_addr - heap->start_addr);

    if(new_size % PAGE_SIZE)
    {
        new_size &= -PAGE_SIZE;
        new_size += PAGE_SIZE;
    }

    if(new_size < KHEAP_MIN_SIZE)
        new_size = KHEAP_MIN_SIZE;

    size_t old_size = heap->end_addr - heap->start_addr;
    size_t i = old_size - PAGE_SIZE;
    while(i >= new_size)
    {
        free_frame(get_page(heap->start_addr + i, 0, kernel_directory));
        i -= PAGE_SIZE;
    }

    heap->end_addr = heap->start_addr + new_size;

    return new_size;
}

void *alloc(size_t bytes, heap_t *heap)
{
    size_t nb = request2size(bytes);

    hchunk_t *hole = find_smallest_chunk(nb, heap);
    if(hole == 0)
    {
        // If we didn't find suitable hole
        // then expand the heap
        size_t old_size = heap->end_addr - heap->start_addr;
        size_t old_end_addr = heap->end_addr;
        expand(old_size + nb, heap);

        size_t new_size = heap->end_addr - heap->start_addr;

        // Find endmost header. (Not endmost in size, but in location).
        struct list_head *tmp = 0;
        if(!list_empty(get_head(heap)))
        {
            struct list_head *iter;
            list_for_each(iter, get_head(heap))
            {
                if(iter > tmp)
                {
                    tmp = iter;
                }
            }
        }

        hchunk_t *topchunk = 0;
        size_t csize = 0;
        if(tmp)
        {
            // Found the endmost free chunk. Check whether we should expand it or not.
            topchunk = list_entry(tmp, hchunk_t, list);
            if(!pinuse(topchunk) && cinuse(topchunk))
            {
                goto _Lassert;
            }

            size_t old_csize = chunksize(topchunk);
            if((size_t)chunk_plus_offset(topchunk, old_csize) != old_end_addr)
            {
                topchunk = 0;
            }
            else
            {
                remove_chunk(topchunk);
                csize = heap->end_addr - (size_t)topchunk;
            }
        }

        if(topchunk == 0)
        {
            // We haven't found free chunk - add one at the end.
            topchunk = (hchunk_t *)old_end_addr;
            csize = heap->end_addr - old_end_addr;
        }

        set_size_and_pinuse(topchunk, csize);
        insert_chunk(topchunk, heap);

        return alloc(bytes, heap);
    }

    ASSERT(chunksize(hole) >= nb);

    // Delete from list
    remove_chunk(hole);

    if(chunksize(hole) >= nb + MIN_CHUNK_SIZE) // We have to split the hole
    {
        // Calculate new hole size
        size_t size = chunksize(hole) - nb;

        // Set header for new chunk
        hchunk_t *new = chunk_plus_offset(hole, nb);
        new->head = size;

        // Add new hole to list
        insert_chunk(new, heap);
    }
    else
    {
        nb = chunksize(hole);
    }

    // Set header
    set_inuse(hole, nb);

    // Return pointer to memory
    return chunk2mem(hole);

_Lassert:
    monitor_write("Kernel Memory Corrupt.");
    PANIC(0);
    return 0;
}

void free(void *ptr, heap_t *heap)
{
    // Standard check if zero pointer passed
    if(ptr == 0)
        return ;

    // Check whether our heap contains ptr
    if(!ok_address(ptr, heap))
    {
        goto _Lassert;
    }

    // Find corresponding chunk
    hchunk_t *block = mem2chunk(ptr);

    // Check inuse flag
    if(!ok_inuse(block))
    {
        goto _Lassert;
    }

    size_t size = chunksize(block);

    // Check for previous chunk. If inuse bit not set,
    // then we can unify both free chunks
    if(!pinuse(block))
    {
        size_t prevsize = block->prev_foot;
        hchunk_t* prev = chunk_minus_offset(block, prevsize);

        if(!ok_address(prev, heap))
        {
            goto _Lassert;
        }

        size += prevsize;
        block = prev;

        // Remove chunk from ordered list
        remove_chunk(block);
    }

    // Find addres of next chunk
    hchunk_t *next = chunk_plus_offset(block, size);
    if(ok_address(next, heap))
    {
        if(!pinuse(next))
        {
            goto _Lassert2;
        }

        // check whether next chunk is inuse or not
        if(cinuse(next))
        {
            // Clear PINUSE bit
            clear_pinuse(next);
        }
        else
        {
            size_t nextsize = chunksize(next);

            if((size_t)chunk_plus_offset(next, nextsize) > heap->end_addr)
            {
                goto _Lassert2;
            }

            // Remove next hole from the list
            remove_chunk(next);

            // And unify it with ours
            size += nextsize;
        }
    }

    // If current hole is the endmost one - we can contract
    if((size_t)chunk_plus_offset(block, size) == heap->end_addr)
    {
        size_t old_size = heap->end_addr - heap->start_addr;
        size_t new_size = (size_t)block - heap->start_addr;

        // Check header corruption
        if(new_size % PAGE_SIZE)
        {
            new_size += MIN_CHUNK_SIZE;
        }
        new_size = contract(new_size, heap);

        if(size > old_size - new_size)
        {
            // We are still exist, so resize chunk
            size -= old_size - new_size;
        }
        else
        {
            // We are no longer exist.
            size = 0;
            block = 0;
        }
    }
    else
    {
        // We should set footer for non-endmost chunk
        set_foot(block, size);
    }


    if(block && size)
    {
        // Add new free chunk to list
        set_size_and_pinuse(block, size);
        insert_chunk(block, heap);
    }

    return ;

_Lassert2:
    insert_chunk(block, heap);
_Lassert:
    monitor_write("Kernel Memory Corrupt.");
    PANIC(0);
}

void *paligned_alloc(size_t bytes, heap_t *heap)
{
    size_t nb = request2size(bytes);
    size_t req = nb + PAGE_SIZE + MIN_CHUNK_SIZE - CHUNK_OVERHEAD;

    void *mem = alloc(req, heap);
    if(mem)
    {
        hchunk_t *chunk = mem2chunk(mem);
        if((size_t)mem % PAGE_SIZE)
        {
            // misaligned

            /*
               Find an aligned spot inside chunk. Since we need to give
               back leading space in a chunk of at least MIN_CHUNK_SIZE, if
               the first calculation places us at a spot with less than
               MIN_CHUNK_SIZE leader, we can move to the next aligned spot.
               We've allocated enough total room so that this is always
               possible.
             */
            char *br = (char*)mem2chunk(((size_t)mem & -PAGE_SIZE) + PAGE_SIZE);
            char *pos = ((size_t)(br - (char*)(chunk)) >= MIN_CHUNK_SIZE) ?
                br : br + PAGE_SIZE;

            hchunk_t *newchunk = (hchunk_t*)pos;
            size_t leadsize = pos - (char*)(chunk);
            size_t newsize = chunksize(chunk) - leadsize;

            set_inuse(newchunk, newsize);

            set_size_and_pinuse(chunk, leadsize);
            newchunk->prev_foot = leadsize;

            insert_chunk(chunk, heap);

            chunk = newchunk;
        }

        /* Give back apre room at the end */
        size_t csize = chunksize(chunk);
        if(csize > nb + MIN_CHUNK_SIZE)
        {
            size_t remainder_size = csize - nb;
            hchunk_t *remainder = chunk_plus_offset(chunk, nb);
            set_inuse(chunk, nb);
            set_inuse(remainder, remainder_size);
            free(chunk2mem(remainder), heap);
        }

        mem = chunk2mem(chunk);
        ASSERT(chunksize(chunk) >= nb);
        ASSERT(((size_t)mem % PAGE_SIZE) == 0);
        ASSERT(cinuse(chunk));
    }

    return mem;
}
