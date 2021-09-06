/* BCST - Introduction to Computer Systems
 * Author:      yangminz@outlook.com
 * Github:      https://github.com/yangminz/bcst_csapp
 * Bilibili:    https://space.bilibili.com/4564101
 * Zhihu:       https://www.zhihu.com/people/zhao-yang-min
 * This project (code repository and videos) is exclusively owned by yangminz 
 * and shall not be used for commercial and profitting purpose 
 * without yangminz's permission.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include "headers/allocator.h"

/* ------------------------------------- */
/*  Operating System Implemented         */
/* ------------------------------------- */

void os_syscall_brk()
{
    // an empty function
}

uint32_t extend_heap(uint32_t size)
{
    // round up to page alignment
    size = (uint32_t)round_up((uint64_t)size, 4096);
    if (heap_end_vaddr - heap_start_vaddr + size <= HEAP_MAX_SIZE)
    {
        // do brk system call to request pages for heap
        os_syscall_brk();
        heap_end_vaddr += size;
    }
    else
    {
        return 0;
    }

    uint64_t epilogue = get_epilogue();
    set_allocated(epilogue, ALLOCATED);
    set_blocksize(epilogue, 0);

    return size;
}

// Round up to next multiple of n:
// if (x == k * n)
// return x
// else, x = k * n + m and m < n
// return (k + 1) * n
uint64_t round_up(uint64_t x, uint64_t n)
{
    return n * ((x + n - 1) / n);
}

/* ------------------------------------- */
/*  Block Operations                     */
/* ------------------------------------- */

// applicapable for both header & footer
uint32_t get_blocksize(uint64_t header_vaddr)
{
    if (header_vaddr == NIL)
    {
        return 0;
    }

    assert(get_prologue() <= header_vaddr && header_vaddr <= get_epilogue());
    assert((header_vaddr & 0x3) == 0x0);  // header & footer should be 4 bytes alignment

    uint32_t header_value = *(uint32_t *)&heap[header_vaddr];
    return (header_value & 0xFFFFFFF8);
}

// applicapable for both header & footer
void set_blocksize(uint64_t header_vaddr, uint32_t blocksize)
{
    if (header_vaddr == NIL)
    {
        return;
    }

    assert(get_prologue() <= header_vaddr && header_vaddr <= get_epilogue());
    assert((header_vaddr & 0x3) == 0x0);  // header & footer should be 4 bytes alignment
    assert((blocksize & 0x7) == 0x0);   // block size should be 8 bytes aligned
    // for last block, the virtual block size is still 8n
    // we imagine there is a footer in another physical page
    // but it actually does not exist

    *(uint32_t *)&heap[header_vaddr] &= 0x00000007;
    *(uint32_t *)&heap[header_vaddr] |= blocksize;
}

// applicapable for both header & footer
uint32_t get_allocated(uint64_t header_vaddr)
{
    if (header_vaddr == NIL)
    {
        // NULL can be considered as allocated
        return ALLOCATED;
    }

    assert(get_prologue() <= header_vaddr && header_vaddr <= get_epilogue());
    assert((header_vaddr & 0x3) == 0x0);  // header & footer should be 4 bytes alignment

    uint32_t header_value = *(uint32_t *)&heap[header_vaddr];
    return (header_value & 0x1);    
}

// applicapable for both header & footer
void set_allocated(uint64_t header_vaddr, uint32_t allocated)
{
    if (header_vaddr == NIL)
    {
        return;
    }

    assert(get_prologue() <= header_vaddr && header_vaddr <= get_epilogue());
    assert((header_vaddr & 0x3) == 0x0);  // header & footer should be 4 bytes alignment

    *(uint32_t *)&heap[header_vaddr] &= 0xFFFFFFFE;
    *(uint32_t *)&heap[header_vaddr] |= (allocated & 0x1);
}

uint64_t get_payload(uint64_t vaddr)
{
    if (vaddr == NIL)
    {
        return NIL;
    }    
    assert(get_firstblock() <= vaddr && vaddr < get_epilogue());

    // vaddr can be:
    // 1. starting address of the block (8 * n + 4)
    // 2. starting address of the payload (8 * m)
    assert((vaddr & 0x3) == 0);

    // this round up will handle `vaddr == NIL` situation
    return round_up(vaddr, 8);
}

uint64_t get_header(uint64_t vaddr)
{
    if (vaddr == NIL)
    {
        return NIL;
    }    
    assert(get_firstblock() <= vaddr && vaddr < get_epilogue());

    // vaddr can be:
    // 1. starting address of the block (8 * n + 4)
    // 2. starting address of the payload (8 * m)
    assert((vaddr & 0x3) == 0);

    uint64_t payload_vaddr = get_payload(vaddr);

    // NULL block does not have header
    return payload_vaddr == NIL ? NIL : payload_vaddr - 4;
}

uint64_t get_footer(uint64_t vaddr)
{
    if (vaddr == NIL)
    {
        return NIL;
    }    
    assert(get_firstblock() <= vaddr && vaddr < get_epilogue());

    // vaddr can be:
    // 1. starting address of the block (8 * n + 4)
    // 2. starting address of the payload (8 * m)
    assert((vaddr & 0x3) == 0);

    uint64_t header_vaddr = get_header(vaddr);
    uint64_t footer_vaddr = header_vaddr + get_blocksize(header_vaddr) - 4;

    assert(get_firstblock() < footer_vaddr && footer_vaddr < get_epilogue());
    return footer_vaddr;
}

/* ------------------------------------- */
/*  Heap Operations                      */
/* ------------------------------------- */

uint64_t get_nextheader(uint64_t vaddr)
{
    if (vaddr == NIL || is_lastblock(vaddr))
    {
        return NIL;
    }

    assert(get_firstblock() <= vaddr && vaddr < get_lastblock());

    // vaddr can be:
    // 1. starting address of the block (8 * n + 4)
    // 2. starting address of the payload (8 * m)
    uint64_t header_vaddr = get_header(vaddr);
    uint32_t block_size = get_blocksize(header_vaddr);

    uint64_t next_header_vaddr = header_vaddr + block_size;
    assert(get_firstblock() < next_header_vaddr &&
        next_header_vaddr <= get_lastblock());
    
    return next_header_vaddr;
}

uint64_t get_prevheader(uint64_t vaddr)
{
    if (vaddr == NIL || is_firstblock(vaddr))
    {
        return NIL;
    }
    
    assert(get_firstblock() < vaddr && vaddr <= get_lastblock());

    uint64_t header_vaddr = get_header(vaddr);

    uint64_t prev_footer_vaddr = header_vaddr - 4;
    uint32_t prev_blocksize = get_blocksize(prev_footer_vaddr);

    uint64_t prev_header_vaddr = header_vaddr - prev_blocksize;
    assert(get_firstblock() <= prev_header_vaddr &&
        prev_header_vaddr <= get_lastblock());
    assert(get_blocksize(prev_header_vaddr) == get_blocksize(prev_footer_vaddr));
    assert(get_allocated(prev_header_vaddr) == get_allocated(prev_footer_vaddr));
    
    return prev_header_vaddr;
}

uint64_t get_firstblock()
{
    assert(heap_end_vaddr > heap_start_vaddr);
    assert((heap_end_vaddr - heap_start_vaddr) % 4096 == 0);
    assert(heap_start_vaddr % 4096 == 0);

    // 4 for the not in use
    // 8 for the prologue block
    return get_prologue() + 8;
}

uint64_t get_lastblock()
{
    assert(heap_end_vaddr > heap_start_vaddr);
    assert((heap_end_vaddr - heap_start_vaddr) % 4096 == 0);
    assert(heap_start_vaddr % 4096 == 0);

    uint64_t epilogue_header = get_epilogue();
    uint64_t last_footer = epilogue_header - 4;
    uint32_t last_blocksize = get_blocksize(last_footer);

    uint64_t last_header = epilogue_header - last_blocksize;

    assert(get_firstblock() <= last_header);

    return last_header;
}

uint64_t get_prologue()
{
    assert(heap_end_vaddr > heap_start_vaddr);
    assert((heap_end_vaddr - heap_start_vaddr) % 4096 == 0);
    assert(heap_start_vaddr % 4096 == 0);

    // 4 for the not in use
    return heap_start_vaddr + 4;
}

uint64_t get_epilogue()
{
    assert(heap_end_vaddr > heap_start_vaddr);
    assert((heap_end_vaddr - heap_start_vaddr) % 4096 == 0);
    assert(heap_start_vaddr % 4096 == 0);

    // epilogue block is having header only
    return heap_end_vaddr - 4;
}

int is_firstblock(uint64_t vaddr)
{
    if (vaddr == NIL)
    {
        return 0;
    }

    // vaddr can be:
    // 1. starting address of the block (8 * n + 4)
    // 2. starting address of the payload (8 * m)
    assert(get_firstblock() <= vaddr && vaddr < get_epilogue());
    assert((vaddr & 0x3) == 0x0);

    uint64_t header_vaddr = get_header(vaddr);
    
    if (header_vaddr == get_firstblock())
    {
        // it is the last block
        // it does not have any footer
        return 1;
    }

    // no, it's not the last block
    // it should have footer
    return 0;
}

int is_lastblock(uint64_t vaddr)
{
    if (vaddr == NIL)
    {
        return 0;
    }

    // vaddr can be:
    // 1. starting address of the block (8 * n + 4)
    // 2. starting address of the payload (8 * m)
    assert(get_firstblock() <= vaddr && vaddr < get_epilogue());
    assert((vaddr & 0x3) == 0x0);

    uint64_t header_vaddr = get_header(vaddr);
    uint32_t blocksize = get_blocksize(header_vaddr);

    if (header_vaddr + blocksize == get_epilogue())
    {
        // it is the last block
        // it does not have any footer
        return 1;
    }

    // no, it's not the last block
    // it should have footer
    return 0;
}

/* ------------------------------------- */
/*  Free Block as Data Structure         */
/* ------------------------------------- */

uint64_t get_field32_block_ptr(uint64_t header_vaddr, uint32_t min_blocksize, uint32_t offset)
{
    if (header_vaddr == NIL)
    {
        return NIL;
    }

    assert(get_firstblock() <= header_vaddr && header_vaddr <= get_lastblock());
    assert(header_vaddr % 8 == 4);
    assert(get_blocksize(header_vaddr) >= min_blocksize);

    assert(offset % 4 == 0);

    uint32_t vaddr_32 = *(uint32_t *)&heap[header_vaddr + offset];
    return (uint64_t)vaddr_32;
}

void set_field32_block_ptr(uint64_t header_vaddr, uint64_t block_ptr, uint32_t min_blocksize, uint32_t offset)
{
    if (header_vaddr == NIL)
    {
        return;
    }
    
    assert(get_firstblock() <= header_vaddr && header_vaddr <= get_lastblock());
    assert(header_vaddr % 8 == 4);
    assert(get_blocksize(header_vaddr) >= min_blocksize);

    assert(block_ptr == NIL || (get_firstblock() <= block_ptr && block_ptr <= get_lastblock()));
    assert(block_ptr % 8 == 4);
    assert(get_blocksize(block_ptr) >= min_blocksize);

    assert(offset % 4 == 0);

    // actually a 32-bit pointer
    assert((block_ptr >> 32) == 0);
    *(uint32_t *)&heap[header_vaddr + offset] = (uint32_t)(block_ptr & 0xFFFFFFFF);
}

/* ------------------------------------- */
/*  Malloc and Free                      */
/* ------------------------------------- */

uint64_t merge_blocks_as_free(uint64_t low, uint64_t high)
{
    assert(low % 8 == 4);
    assert(high % 8 == 4);
    assert(get_firstblock() <= low && low < get_lastblock());
    assert(get_firstblock() < high && high <= get_lastblock());
    assert(get_nextheader(low) == high);
    assert(get_prevheader(high) == low);

    // must merge as free
    uint32_t blocksize = get_blocksize(low) + get_blocksize(high);

    set_blocksize(low, blocksize);
    set_allocated(low, FREE);

    uint64_t footer = get_footer(high);
    set_blocksize(footer, blocksize);
    set_allocated(footer, FREE);

    return low;
}

uint64_t try_alloc_with_splitting(uint64_t block_vaddr, uint32_t request_blocksize, uint32_t min_blocksize)
{
    if (request_blocksize < min_blocksize)
    {
        return NIL;
    }

    uint64_t b = block_vaddr;

    uint32_t b_blocksize = get_blocksize(b);
    uint32_t b_allocated = get_allocated(b);

    if (b_allocated == FREE && b_blocksize >= request_blocksize)
    {
        // allocate this block
        if (b_blocksize - request_blocksize >= min_blocksize)
        {
            // split this block `b`
            // b_blocksize - request_blocksize >= 8
            uint64_t next_footer = get_footer(b);
            set_allocated(next_footer, FREE);
            set_blocksize(next_footer, b_blocksize - request_blocksize);

            set_allocated(b, ALLOCATED);
            set_blocksize(b, request_blocksize);

            uint64_t b_footer = get_footer(b);
            set_allocated(b_footer, ALLOCATED);
            set_blocksize(b_footer, request_blocksize);

            // set the left splitted block
            // in the extreme situation, next block size == 8
            // which makes the whole block of next to be:
            // [0x00000008, 0x00000008]
            uint64_t next_header = get_nextheader(b);
            set_allocated(next_header, FREE);
            set_blocksize(next_header, b_blocksize - request_blocksize);

            assert(get_footer(next_header) == next_footer);

            return get_payload(b);
        }
        else if (b_blocksize - request_blocksize < min_blocksize)
        {
            // TODO: potential optimization: reduce the fragmentation
            // no need to split this block
            // set_blocksize(b, request_blocksize);
            set_allocated(b, ALLOCATED);
            uint64_t b_footer = get_footer(b);
            set_allocated(b_footer, ALLOCATED);

            return get_payload(b);
        }
    }

    return NIL;
}

uint64_t try_extend_heap_to_alloc(uint32_t size, uint32_t min_blocksize)
{
    // get the size to be added
    uint64_t old_last = get_lastblock();

    uint32_t last_allocated = get_allocated(old_last);
    uint32_t last_blocksize = get_blocksize(old_last);

    uint32_t to_request_from_OS = size;
    if (last_allocated == FREE)
    {
        // last block can help the request
        to_request_from_OS -= last_blocksize;
    }

    uint64_t old_epilogue = get_epilogue();

    uint32_t os_allocated_size = extend_heap(to_request_from_OS);
    if (os_allocated_size != 0)
    {
        assert(os_allocated_size >= 4096);
        assert(os_allocated_size % 4096 == 0);

        uint64_t payload_header = NIL;

        // now last block is different
        // but we check the old last block
        if (last_allocated == ALLOCATED)
        {
            // no merging is needed
            // take place the old epilogue as new last 
            uint64_t new_last = old_epilogue;
            set_allocated(new_last, FREE);
            set_blocksize(new_last, os_allocated_size);

            // set the new footer
            uint64_t new_last_footer = get_footer(new_last);
            set_allocated(new_last_footer, FREE);
            set_blocksize(new_last_footer, os_allocated_size);

            payload_header = new_last;
        }
        else
        {
            // merging with last_block is needed
            set_allocated(old_last, FREE);
            set_blocksize(old_last, last_blocksize + os_allocated_size);

            uint64_t last_footer = get_footer(old_last);
            set_allocated(last_footer, FREE);
            set_blocksize(last_footer, last_blocksize + os_allocated_size);

            payload_header = old_last;
        }

        // try to allocate
        uint64_t payload_vaddr = try_alloc_with_splitting(payload_header, size, min_blocksize);

        if (payload_vaddr != NIL)
        {
#ifdef DEBUG_MALLOC
            check_heap_correctness();
#endif
            return payload_vaddr;
        }
    }

    // else, no page can be allocated
#ifdef DEBUG_MALLOC
    check_heap_correctness();
    printf("OS cannot allocate physical page for heap!\n");
#endif

    return NIL;
}

/* ------------------------------------- */
/*  Debugging and Correctness Checking   */
/* ------------------------------------- */

void check_heap_correctness()
{
    int linear_free_counter = 0;
    uint64_t p = get_firstblock();
    while(p != NIL && p <= get_lastblock())
    {
        assert(p % 8 == 4);
        assert(get_firstblock() <= p && p <= get_lastblock());

        uint64_t f = get_footer(p);
        assert(get_blocksize(p) == get_blocksize(f));
        assert(get_allocated(p) == get_allocated(f));

        // rule 1: block[0] ==> A/F
        // rule 2: block[-1] ==> A/F
        // rule 3: block[i] == A ==> block[i-1] == A/F && block[i+1] == A/F
        // rule 4: block[i] == F ==> block[i-1] == A && block[i+1] == A
        // these 4 rules ensures that
        // adjacent free blocks are always merged together
        // henceforth external fragmentation are minimized
        if (get_allocated(p) == FREE)
        {
            linear_free_counter += 1;
        }
        else
        {
            linear_free_counter = 0;
        }
        assert(linear_free_counter <= 1);

        p = get_nextheader(p);
    }
}

void print_heap()
{
    printf("============\nheap blocks:\n");
    uint64_t h = get_firstblock();
    int i = 0;
    while (h != NIL && h < get_epilogue())
    {
        uint32_t a = get_allocated(h);
        uint32_t s = get_blocksize(h);
        uint64_t f = get_footer(h);

        printf("[H:%lu,F:%lu,S:%u,A:%u]  ", h, f, s, a);
        h = get_nextheader(h);

        i += 1;
        if (i % 5 == 0)
        {
            printf("\b\n");
        }
    }
    printf("\b\b\n============\n");
}

#ifdef DEBUG_MALLOC
#include "headers/algorithm.h"

/* ------------------------------------- */
/*  Unit tests                           */
/* ------------------------------------- */

static void test_roundup()
{
    printf("Testing round up ...\n");

    for (int i = 0; i < 100; ++ i)
    {
        for (int j = 1; j <= 8; ++ j)
        {
            uint32_t x = i * 8 + j;
            assert(round_up(x, 8) == (i + 1) * 8);
        }
    }

    printf("\033[32;1m\tPass\033[0m\n");
}

/*  hex table
    0       0000    *
    1       0001    *
    2       0010
    3       0011
    4       0100
    5       0101
    6       0110
    7       0111
    8       1000    *
    9       1001    *
    a   10  1010
    b   11  1011
    c   12  1100
    d   13  1101
    e   14  1110
    f   15  1111
 */
static void test_get_blocksize_allocated()
{
    printf("Testing getting block size from header ...\n");

    heap_init();

    for (int i = get_prologue(); i <= get_epilogue(); i += 4)
    {
        *(uint32_t *)&heap[i] = 0x1234abc0;
        assert(get_blocksize(i) == 0x1234abc0);
        assert(get_allocated(i) == FREE);

        *(uint32_t *)&heap[i] = 0x1234abc1;
        assert(get_blocksize(i) == 0x1234abc0);
        assert(get_allocated(i) == ALLOCATED);

        *(uint32_t *)&heap[i] = 0x1234abc8;
        assert(get_blocksize(i) == 0x1234abc8);
        assert(get_allocated(i) == FREE);

        *(uint32_t *)&heap[i] = 0x1234abc9;
        assert(get_blocksize(i) == 0x1234abc8);
        assert(get_allocated(i) == ALLOCATED);
    }

    printf("\033[32;1m\tPass\033[0m\n");
}

static void test_set_blocksize_allocated()
{
    printf("Testing setting block size to header ...\n");

    heap_init();

    for (int i = get_prologue(); i <= get_epilogue(); i += 4)
    {
        set_blocksize(i, 0x1234abc0);
        set_allocated(i, FREE);
        assert(get_blocksize(i) == 0x1234abc0);
        assert(get_allocated(i) == FREE);

        set_blocksize(i, 0x1234abc0);
        set_allocated(i, ALLOCATED);
        assert(get_blocksize(i) == 0x1234abc0);
        assert(get_allocated(i) == ALLOCATED);

        set_blocksize(i, 0x1234abc8);
        set_allocated(i, FREE);
        assert(get_blocksize(i) == 0x1234abc8);
        assert(get_allocated(i) == FREE);

        set_blocksize(i, 0x1234abc8);
        set_allocated(i, ALLOCATED);
        assert(get_blocksize(i) == 0x1234abc8);
        assert(get_allocated(i) == ALLOCATED);
    }

    // set the block size of last block
    for (int i = 2; i < 100; ++ i)
    {
        uint32_t blocksize = i * 8;
        uint64_t addr = get_epilogue() - blocksize;   // + 4 for the virtual footer in next page
        set_blocksize(addr, blocksize);
        assert(get_blocksize(addr) == blocksize);
        assert(is_lastblock(addr) == 1);
    }

    printf("\033[32;1m\tPass\033[0m\n");
}

static void test_get_header_payload_addr()
{
    printf("Testing getting header or payload virtual addresses ...\n");

    heap_init();

    uint64_t header_addr, payload_addr;
    for (int i = get_payload(get_firstblock()); i < get_epilogue(); i += 8)
    {
        payload_addr = i;
        header_addr = payload_addr - 4;

        assert(get_payload(header_addr) == payload_addr);
        assert(get_payload(payload_addr) == payload_addr);

        assert(get_header(header_addr) == header_addr);
        assert(get_header(payload_addr) == header_addr);
    }

    printf("\033[32;1m\tPass\033[0m\n");
}

static void test_get_next_prev()
{
    printf("Testing linked list traversal ...\n");

    srand(123456);

    // let me setup the heap first
    heap_init();

    uint64_t h = get_firstblock();
    uint64_t f = NIL;

    uint32_t collection_blocksize[1000];
    uint32_t collection_allocated[1000];
    uint32_t collection_headeraddr[1000];
    int counter = 0;

    uint32_t allocated = 1;
    uint64_t epilogue = get_epilogue();
    while (h < epilogue)
    {
        uint32_t blocksize = 8 * (1 + rand() % 16);
        if (epilogue - h <= 64)
        {
            blocksize = epilogue - h;
        }

        if (allocated == ALLOCATED && (rand() % 3) >= 1)
        {
            // with previous allocated, 2/3 possibilities to be free
            allocated = FREE;
        }
        else
        {
            allocated = ALLOCATED;
        }

        collection_blocksize[counter] = blocksize;
        collection_allocated[counter] = allocated;
        collection_headeraddr[counter] = h;
        counter += 1;

        set_blocksize(h, blocksize);
        set_allocated(h, allocated);

        f = h + blocksize - 4;
        set_blocksize(f, blocksize);
        set_allocated(f, allocated);

        h = h + blocksize;
    }
    
    // check get_next
    h = get_firstblock();
    int i = 0;
    while (h != NIL && h < get_epilogue())
    {
        assert(i <= counter);
        assert(h == collection_headeraddr[i]);
        assert(get_blocksize(h) == collection_blocksize[i]);
        assert(get_allocated(h) == collection_allocated[i]);
        
        h = get_nextheader(h);
        i += 1;
    }

    check_heap_correctness();

    // check get_prev
    h = get_lastblock();
    i = counter - 1;
    while (h != NIL && get_firstblock() <= h)
    {
        assert(0 <= i);
        assert(h == collection_headeraddr[i]);
        assert(get_blocksize(h) == collection_blocksize[i]);
        assert(get_allocated(h) == collection_allocated[i]);

        h = get_prevheader(h);
        i -= 1;
    }

    printf("\033[32;1m\tPass\033[0m\n");
}

static void test_malloc_free()
{
    printf("Testing implicit list malloc & free ...\n");

    heap_init();
    check_heap_correctness();

    srand(123456);
    
    // collection for the pointers
    linkedlist_t *ptrs = linkedlist_construct();

    for (int i = 0; i < 100000; ++ i)
    {
        if ((rand() & 0x1) == 0)
        {
            // malloc
            uint32_t size = rand() % 1024 + 1;  // a non zero value
#ifdef DEBUG_MALLOC
            sprintf(debug_message, "[%d] mem_malloc(%u)", i, size);
#endif
            uint64_t p = mem_alloc(size);

            if (p != NIL)
            {
                ptrs = linkedlist_add(ptrs, p);
            }
        }
        else if (ptrs->count != 0)
        {
            // free
            // randomly select one to free
            int random_index = rand() % ptrs->count;
            linkedlist_node_t *t = linkedlist_index(ptrs, random_index);
#ifdef DEBUG_MALLOC
            sprintf(debug_message, "[%d] mem_free(%lu)", i, t->value);
#endif
            mem_free(t->value);
            linkedlist_delete(ptrs, t);
        }
    }

    int num_still_allocated = ptrs->count;
    for (int i = 0; i < num_still_allocated; ++ i)
    {
        linkedlist_node_t *t = linkedlist_next(ptrs);
        mem_free(t->value);
        int x = linkedlist_delete(ptrs, t);
    }
    assert(ptrs->count == 0);
    linkedlist_free(ptrs);

    // finally there should be only one free block
    assert(is_lastblock(get_firstblock()) == 1);
    assert(get_allocated(get_firstblock()) == FREE);
    check_heap_correctness();

    printf("\033[32;1m\tPass\033[0m\n");
}

int main()
{
    test_roundup();
    test_get_blocksize_allocated();
    test_set_blocksize_allocated();
    test_get_header_payload_addr();
    test_get_next_prev();
    
    test_malloc_free();

    return 0;
}

#endif