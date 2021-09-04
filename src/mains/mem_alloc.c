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
#include "headers/algorithm.h"

int heap_init();
uint64_t mem_alloc(uint32_t size);
void mem_free(uint64_t vaddr);

static int implicit_free_list_heap_init();
static uint64_t implicit_free_list_mem_alloc(uint32_t size);
static void implicit_free_list_mem_free(uint64_t payload_vaddr);

static int explicit_free_list_heap_init();
static uint64_t explicit_free_list_mem_alloc(uint32_t size);
static void explicit_free_list_mem_free(uint64_t payload_vaddr);

static int bst_heap_init();
static uint64_t bst_mem_alloc(uint32_t size);
static void bst_mem_free(uint64_t payload_vaddr);

// to allocate one physical page for heap
static uint32_t extend_heap(uint32_t size);
void os_syscall_brk()
{
    // an empty function
}

// heap's bytes range:
// [heap_start_vaddr, heap_end_vaddr) or [heap_start_vaddr, heap_end_vaddr - 1]
// [0,1,2,3] - unused
// [4,5,6,7,8,9,10,11] - prologue block
// [12, ..., 4096 * n - 5] - regular blocks
// 4096 * n + [- 4, -3, -2, -1] - epilogue block (header only)
uint64_t heap_start_vaddr = 0;  // for unit test convenience
uint64_t heap_end_vaddr = 4096;

#define HEAP_MAX_SIZE (4096 * 8)
uint8_t heap[HEAP_MAX_SIZE];

static uint64_t round_up(uint64_t x, uint64_t n);

static uint32_t get_blocksize(uint64_t header_vaddr);
static void set_blocksize(uint64_t header_vaddr, uint32_t blocksize);

static uint32_t get_allocated(uint64_t header_vaddr);
static void set_allocated(uint64_t header_vaddr, uint32_t allocated);

static uint64_t get_prologue();
static uint64_t get_epilogue();

static uint64_t get_firstblock();
static uint64_t get_lastblock();

static int is_lastblock(uint64_t vaddr);
static int is_firstblock(uint64_t vaddr);

static uint64_t get_payload(uint64_t vaddr);
static uint64_t get_header(uint64_t vaddr);
static uint64_t get_footer(uint64_t vaddr);

static uint64_t get_nextheader(uint64_t vaddr);
static uint64_t get_prevheader(uint64_t vaddr);

static void check_heap_correctness();

static void print_heap();

#ifdef DEBUG_MALLOC
#define MAX_LINE_MESSAGE (5)
char debug_message[1000];

void on_sigabrt(int signum)
{
    // like a try-catch for the asserts
    printf("%s\n", debug_message);
    print_heap();
}
#endif

// Round up to next multiple of n:
// if (x == k * n)
// return x
// else, x = k * n + m and m < n
// return (k + 1) * n
static uint64_t round_up(uint64_t x, uint64_t n)
{
    return n * ((x + n - 1) / n);
}

static uint32_t extend_heap(uint32_t size)
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
    set_allocated(epilogue, 1);
    set_blocksize(epilogue, 0);

    return size;
}

// applicapable for both header & footer
static uint32_t get_blocksize(uint64_t header_vaddr)
{
    if (header_vaddr == 0)
    {
        return 0;
    }

    assert(get_prologue() <= header_vaddr && header_vaddr <= get_epilogue());
    assert((header_vaddr & 0x3) == 0x0);  // header & footer should be 4 bytes alignment

    uint32_t header_value = *(uint32_t *)&heap[header_vaddr];
    return (header_value & 0xFFFFFFF8);
}

// applicapable for both header & footer
static void set_blocksize(uint64_t header_vaddr, uint32_t blocksize)
{
    if (header_vaddr == 0)
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
static uint32_t get_allocated(uint64_t header_vaddr)
{
    if (header_vaddr == 0)
    {
        // NULL can be considered as allocated
        return 1;
    }

    assert(get_prologue() <= header_vaddr && header_vaddr <= get_epilogue());
    assert((header_vaddr & 0x3) == 0x0);  // header & footer should be 4 bytes alignment

    uint32_t header_value = *(uint32_t *)&heap[header_vaddr];
    return (header_value & 0x1);    
}

// applicapable for both header & footer
static void set_allocated(uint64_t header_vaddr, uint32_t allocated)
{
    if (header_vaddr == 0)
    {
        return;
    }

    assert(get_prologue() <= header_vaddr && header_vaddr <= get_epilogue());
    assert((header_vaddr & 0x3) == 0x0);  // header & footer should be 4 bytes alignment

    *(uint32_t *)&heap[header_vaddr] &= 0xFFFFFFF8;
    *(uint32_t *)&heap[header_vaddr] |= (allocated & 0x1);
}

static uint64_t get_firstblock()
{
    assert(heap_end_vaddr > heap_start_vaddr);
    assert((heap_end_vaddr - heap_start_vaddr) % 4096 == 0);
    assert(heap_start_vaddr % 4096 == 0);

    // 4 for the not in use
    // 8 for the prologue block
    return get_prologue() + 8;
}

static uint64_t get_lastblock()
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

static uint64_t get_prologue()
{
    assert(heap_end_vaddr > heap_start_vaddr);
    assert((heap_end_vaddr - heap_start_vaddr) % 4096 == 0);
    assert(heap_start_vaddr % 4096 == 0);

    // 4 for the not in use
    return heap_start_vaddr + 4;
}

static uint64_t get_epilogue()
{
    assert(heap_end_vaddr > heap_start_vaddr);
    assert((heap_end_vaddr - heap_start_vaddr) % 4096 == 0);
    assert(heap_start_vaddr % 4096 == 0);

    // epilogue block is having header only
    return heap_end_vaddr - 4;
}

static int is_firstblock(uint64_t vaddr)
{
    if (vaddr == 0)
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

static int is_lastblock(uint64_t vaddr)
{
    if (vaddr == 0)
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

static uint64_t get_payload(uint64_t vaddr)
{
    if (vaddr == 0)
    {
        return 0;
    }    
    assert(get_firstblock() <= vaddr && vaddr < get_epilogue());

    // vaddr can be:
    // 1. starting address of the block (8 * n + 4)
    // 2. starting address of the payload (8 * m)
    assert((vaddr & 0x3) == 0);

    // this round up will handle `vaddr == 0` situation
    return round_up(vaddr, 8);
}

static uint64_t get_header(uint64_t vaddr)
{
    if (vaddr == 0)
    {
        return 0;
    }    
    assert(get_firstblock() <= vaddr && vaddr < get_epilogue());

    // vaddr can be:
    // 1. starting address of the block (8 * n + 4)
    // 2. starting address of the payload (8 * m)
    assert((vaddr & 0x3) == 0);

    uint64_t payload_vaddr = get_payload(vaddr);

    // NULL block does not have header
    return payload_vaddr == 0 ? 0 : payload_vaddr - 4;
}

static uint64_t get_footer(uint64_t vaddr)
{
    if (vaddr == 0)
    {
        return 0;
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

static uint64_t get_nextheader(uint64_t vaddr)
{
    if (vaddr == 0 || is_lastblock(vaddr))
    {
        return 0;
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

static uint64_t get_prevheader(uint64_t vaddr)
{
    if (vaddr == 0 || is_firstblock(vaddr))
    {
        return 0;
    }
    
    assert(get_firstblock() < vaddr && vaddr <= get_lastblock());

    uint64_t header_vaddr = get_header(vaddr);

    uint64_t prev_footer_vaddr = header_vaddr - 4;
    uint32_t prev_blocksize = get_blocksize(prev_footer_vaddr);

    uint64_t prev_header_vaddr = header_vaddr - prev_blocksize;
    assert(get_firstblock() <= prev_header_vaddr &&
        prev_header_vaddr <= get_lastblock());
    assert(*(uint32_t *)&heap[prev_header_vaddr] == *(uint32_t *)&heap[prev_footer_vaddr]);
    
    return prev_header_vaddr;
}

static uint64_t merge_blocks_as_free(uint64_t low, uint64_t high)
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
    set_allocated(low, 0);

    uint64_t footer = get_footer(high);
    set_blocksize(footer, blocksize);
    set_allocated(footer, 0);

    return low;
}

static uint64_t try_alloc_with_splitting(uint64_t block_vaddr, uint32_t request_blocksize, uint32_t min_blocksize)
{
    if (request_blocksize < min_blocksize)
    {
        return 0;
    }

    uint64_t b = block_vaddr;

    uint32_t b_blocksize = get_blocksize(b);
    uint32_t b_allocated = get_allocated(b);

    if (b_allocated == 0 && b_blocksize >= request_blocksize)
    {
        // allocate this block
        if (b_blocksize - request_blocksize >= min_blocksize)
        {
            // split this block `b`
            // b_blocksize - request_blocksize >= 8
            uint64_t next_footer = get_footer(b);
            set_allocated(next_footer, 0);
            set_blocksize(next_footer, b_blocksize - request_blocksize);

            set_allocated(b, 1);
            set_blocksize(b, request_blocksize);

            uint64_t b_footer = get_footer(b);
            set_allocated(b_footer, 1);
            set_blocksize(b_footer, request_blocksize);

            // set the left splitted block
            // in the extreme situation, next block size == 8
            // which makes the whole block of next to be:
            // [0x00000008, 0x00000008]
            uint64_t next_header = get_nextheader(b);
            set_allocated(next_header, 0);
            set_blocksize(next_header, b_blocksize - request_blocksize);

            assert(get_footer(next_header) == next_footer);

            return get_payload(b);
        }
        else if (b_blocksize - request_blocksize < min_blocksize)
        {
            // TODO: potential optimization: reduce the padding size
            // no need to split this block
            // set_blocksize(b, request_blocksize);
            set_allocated(b, 1);
            uint64_t b_footer = get_footer(b);
            set_allocated(b_footer, 1);

            return get_payload(b);
        }
    }

    return 0;
}

static uint64_t brk_when_almost_full(uint32_t size, uint32_t min_blocksize)
{
    // get the size to be added
    uint64_t old_last = get_lastblock();

    uint32_t last_allocated = get_allocated(old_last);
    uint32_t last_blocksize = get_blocksize(old_last);

    uint32_t to_request_from_OS = size;
    if (last_allocated == 0)
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

        uint64_t payload_header = 0;

        // now last block is different
        // but we check the old last block
        if (last_allocated == 1)
        {
            // no merging is needed
            // take place the old epilogue as new last 
            uint64_t new_last = old_epilogue;
            set_allocated(new_last, 0);
            set_blocksize(new_last, os_allocated_size);

            // set the new footer
            uint64_t new_last_footer = get_footer(new_last);
            set_allocated(new_last_footer, 0);
            set_blocksize(new_last_footer, os_allocated_size);

            payload_header = new_last;
        }
        else
        {
            // merging with last_block is needed
            set_allocated(old_last, 0);
            set_blocksize(old_last, last_blocksize + os_allocated_size);

            uint64_t last_footer = get_footer(old_last);
            set_allocated(last_footer, 0);
            set_blocksize(last_footer, last_blocksize + os_allocated_size);

            payload_header = old_last;
        }

        // try to allocate
        uint64_t payload_vaddr = try_alloc_with_splitting(payload_header, size, min_blocksize);

        if (payload_vaddr != 0)
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

    return 0;
}

/* ------------------------------------- */
/*  Correctness Checking                 */
/* ------------------------------------- */

static void check_heap_correctness()
{
    int linear_free_counter = 0;
    uint64_t p = get_firstblock();
    while(p != 0 && p <= get_lastblock())
    {
        assert(p % 8 == 4);
        assert(get_firstblock() <= p && p <= get_lastblock());

        assert(*(uint32_t *)&heap[p] == *(uint32_t *)&heap[get_footer(p)]);

        // rule 1: block[0] ==> A/F
        // rule 2: block[-1] ==> A/F
        // rule 3: block[i] == A ==> block[i-1] == A/F && block[i+1] == A/F
        // rule 4: block[i] == F ==> block[i-1] == A && block[i+1] == A
        // these 4 rules ensures that
        // adjacent free blocks are always merged together
        // henceforth external fragmentation are minimized
        if (get_allocated(p) == 0)
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

/* ------------------------------------- */
/*  Debugging                            */
/* ------------------------------------- */

static void print_heap()
{
    printf("============\nheap blocks:\n");
    uint64_t h = get_firstblock();
    int i = 0;
    while (h != 0 && h < get_epilogue())
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

/* ------------------------------------- */
/*  Exposed Interface                    */
/* ------------------------------------- */

int heap_init()
{
    return implicit_free_list_heap_init();
}

uint64_t mem_alloc(uint32_t size)
{
    return implicit_free_list_mem_alloc(size);
}

void mem_free(uint64_t payload_vaddr)
{
    implicit_free_list_mem_free(payload_vaddr);
}


/* ------------------------------------- */
/*  Implicit Free List                   */
/* ------------------------------------- */

/*  Allocated block:

    ff ff ff f9/f1  [8n + 24] - footer
    ?? ?? ?? ??     [8n + 20] - padding
    xx xx ?? ??     [8n + 16] - payload & padding
    xx xx xx xx     [8n + 12] - payload
    xx xx xx xx     [8n + 8] - payload
    hh hh hh h9/h1  [8n + 4] - header

    Free block:

    ff ff ff f8/f0  [8n + 24] - footer
    ?? ?? ?? ??     [8n + 20]
    ?? ?? ?? ??     [8n + 16]
    ?? ?? ?? ??     [8n + 12]
    ?? ?? ?? ??     [8n + 8]
    hh hh hh h8/h0  [8n + 4] - header
*/

#define MIN_IMPLICIT_FREE_LIST_BLOCKSIZE (8)

static int implicit_free_list_heap_init()
{
    // reset all to 0
    for (int i = 0; i < HEAP_MAX_SIZE / 8; i += 8)
    {
        *(uint64_t *)&heap[i] = 0;
    }

    // heap_start_vaddr is the starting address of the first block
    // the payload of the first block is 8B aligned ([8])
    // so the header address of the first block is [8] - 4 = [4]
    heap_start_vaddr = 0;
    heap_end_vaddr = 4096;

    // set the prologue block
    uint64_t prologue_header = get_prologue();
    set_blocksize(prologue_header, 8);
    set_allocated(prologue_header, 1);

    uint64_t prologue_footer = prologue_header + 4;
    set_blocksize(prologue_footer, 8);
    set_allocated(prologue_footer, 1);

    // set the epilogue block
    // it's a footer only
    uint64_t epilogue = get_epilogue();
    set_blocksize(epilogue, 0);
    set_allocated(epilogue, 1);

    // set the block size & allocated of the only regular block
    uint64_t first_header = get_firstblock();
    set_blocksize(first_header, 4096 - 4 - 8 - 4);
    set_allocated(first_header, 0);

    uint64_t first_footer = get_footer(first_header);
    set_blocksize(first_footer, 4096 - 4 - 8 - 4);
    set_allocated(first_footer, 0);

#ifdef DEBUG_MALLOC
    // like a try-catch
    signal(SIGABRT, &on_sigabrt);
#endif

    return 1;
}

// size - requested payload size
// return - the virtual address of payload
static uint64_t implicit_free_list_mem_alloc(uint32_t size)
{
#ifdef DEBUG_MALLOC
    sprintf(debug_message, "mem_malloc(%u)", size);
#endif

    assert(0 < size && size < HEAP_MAX_SIZE - 4 - 8 - 4);

    uint64_t payload_vaddr = 0;    
    uint32_t request_blocksize = round_up(size, 8) + 4 + 4;
    request_blocksize = request_blocksize < MIN_IMPLICIT_FREE_LIST_BLOCKSIZE ?
        MIN_IMPLICIT_FREE_LIST_BLOCKSIZE : request_blocksize;

    uint64_t b = get_firstblock();
    uint64_t epilogue = get_epilogue();

    // not thread safe
    while (b != 0 && b < epilogue)
    {
        payload_vaddr = try_alloc_with_splitting(b, request_blocksize, MIN_IMPLICIT_FREE_LIST_BLOCKSIZE);

        if (payload_vaddr != 0)
        {
#ifdef DEBUG_MALLOC
            check_heap_correctness();
#endif
            return payload_vaddr;
        }
        else
        {
            // go to next block
            b = get_nextheader(b);
        }
    }

    // when no enough free block for current heap
    // request a new free physical & virtual page from OS
    return brk_when_almost_full(request_blocksize, MIN_IMPLICIT_FREE_LIST_BLOCKSIZE);
}

static void implicit_free_list_mem_free(uint64_t payload_vaddr)
{
#ifdef DEBUG_MALLOC
    sprintf(debug_message, "mem_free(%lu)", payload_vaddr);
#endif

    if (payload_vaddr == 0)
    {
        return;
    }

    assert(get_firstblock() < payload_vaddr && payload_vaddr < get_epilogue());
    assert((payload_vaddr & 0x7) == 0x0);

    // request can be first or last block
    uint64_t req = get_header(payload_vaddr);
    uint64_t req_footer = get_footer(req); // for last block, it's 0

    uint32_t req_allocated = get_allocated(req);
    uint32_t req_blocksize = get_blocksize(req);
    assert(req_allocated == 1); // otherwise it's free twice

    // block starting address of next & prev blocks
    uint64_t next = get_nextheader(req);    // for req last block, it's 0
    uint64_t prev = get_prevheader(req);    // for req first block, it's 0

    uint32_t next_allocated = get_allocated(next);  // for req last, 1
    uint32_t prev_allocated = get_allocated(prev);  // for req first, 1

    if (next_allocated == 1 && prev_allocated == 1)
    {
        // case 1: *A(A->F)A*
        // ==> *AFA*
        set_allocated(req, 0);
        set_allocated(req_footer, 0);
#ifdef DEBUG_MALLOC
        check_heap_correctness();
#endif
    }
    else if (next_allocated == 0 && prev_allocated == 1)
    {
        // case 2: *A(A->F)FA
        // ==> *AFFA ==> *A[FF]A merge current and next
        merge_blocks_as_free(req, next);
#ifdef DEBUG_MALLOC
        check_heap_correctness();
#endif
    }
    else if (next_allocated == 1 && prev_allocated == 0)
    {
        // case 3: AF(A->F)A*
        // ==> AFFA* ==> A[FF]A* merge current and prev
        merge_blocks_as_free(prev, req);
#ifdef DEBUG_MALLOC
        check_heap_correctness();
#endif
    }
    else if (next_allocated == 0 && prev_allocated == 0)
    {
        // case 4: AF(A->F)FA
        // ==> AFFFA ==> A[FFF]A merge current and prev and next
        uint64_t merged = merge_blocks_as_free(prev, req);
        merge_blocks_as_free(merged, next);
#ifdef DEBUG_MALLOC
        check_heap_correctness();
#endif
    }
    else
    {
#ifdef DEBUG_MALLOC
        printf("exception for free\n");
        exit(0);
#endif
    }
}

/* ------------------------------------- */
/*  Explicit Free List                   */
/* ------------------------------------- */

/*  Allocated block:

    ff ff ff f9/f1  [8n + 24] - footer
    ?? ?? ?? ??     [8n + 20] - padding
    xx xx ?? ??     [8n + 16] - payload & padding
    xx xx xx xx     [8n + 12] - payload
    xx xx xx xx     [8n + 8] - payload
    hh hh hh h9/h1  [8n + 4] - header

    Free block:

    ff ff ff f8/f0  [8n + 24] - footer
    ?? ?? ?? ??     [8n + 20]
    ?? ?? ?? ??     [8n + 16]
    nn nn nn nn     [8n + 12] - next free block address
    pp pp pp pp     [8n + 8] - previous free block address
    hh hh hh h8/h0  [8n + 4] - header
*/
#define MIN_EXPLICIT_FREE_LIST_BLOCKSIZE (16)

static uint64_t explicit_free_list_head = 0;
static uint32_t explicit_free_list_counter = 0;

static uint64_t get_prevfree(uint64_t header_vaddr)
{
    assert(get_firstblock() <= header_vaddr && header_vaddr <= get_lastblock());
    assert(header_vaddr % 8 == 4);
    assert(get_blocksize(header_vaddr) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

    uint32_t vaddr_32 = *(uint32_t *)&heap[header_vaddr + 4];
    return (uint64_t)vaddr_32;
}

static uint64_t get_nextfree(uint64_t header_vaddr)
{
    assert(get_firstblock() <= header_vaddr && header_vaddr <= get_lastblock());
    assert(header_vaddr % 8 == 4);
    assert(get_blocksize(header_vaddr) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

    uint32_t vaddr_32 = *(uint32_t *)&heap[header_vaddr + 8];
    return (uint64_t)vaddr_32;
}

static void set_prevfree(uint64_t header_vaddr, uint64_t prev_vaddr)
{
    assert(get_firstblock() <= header_vaddr && header_vaddr <= get_lastblock());
    assert(header_vaddr % 8 == 4);
    assert(get_blocksize(header_vaddr) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

    assert(get_firstblock() <= prev_vaddr && prev_vaddr <= get_lastblock());
    assert(prev_vaddr % 8 == 4);
    assert(get_blocksize(prev_vaddr) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

    assert((prev_vaddr >> 32) == 0);
    *(uint32_t *)&heap[header_vaddr + 4] = (uint32_t)(prev_vaddr & 0xFFFFFFFF);
}

static void set_nextfree(uint64_t header_vaddr, uint64_t next_vaddr)
{
    assert(get_firstblock() <= header_vaddr && header_vaddr <= get_lastblock());
    assert(header_vaddr % 8 == 4);
    assert(get_blocksize(header_vaddr) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

    assert(get_firstblock() <= next_vaddr && next_vaddr <= get_lastblock());
    assert(next_vaddr % 8 == 4);
    assert(get_blocksize(next_vaddr) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

    assert((next_vaddr >> 32) == 0);
    *(uint32_t *)&heap[header_vaddr + 8] = (uint32_t)(next_vaddr & 0xFFFFFFFF);
}

static void explicit_free_list_insert(uint64_t block)
{
    assert(get_firstblock() <= block && block <= get_lastblock());
    assert(block % 8 == 4);
    assert(get_blocksize(block) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

    if (explicit_free_list_head == 0 || explicit_free_list_counter == 0)
    {
        assert(explicit_free_list_head == 0);
        assert(explicit_free_list_counter == 0);

        set_prevfree(block, block);
        set_nextfree(block, block);

        explicit_free_list_head = block;
        explicit_free_list_counter = 1;

        return;
    }

    // list is not empty
    uint64_t head = explicit_free_list_head;
    uint64_t tail = get_prevfree(head);

    set_nextfree(block, head);
    set_prevfree(head, block);
    set_nextfree(tail, block);
    set_prevfree(block, tail);

    explicit_free_list_head = block;
    explicit_free_list_counter += 1;
}

static void explicit_free_list_delete(uint64_t block)
{
    assert(get_firstblock() <= block && block <= get_lastblock());
    assert(block % 8 == 4);
    assert(get_blocksize(block) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

    if (explicit_free_list_head == 0 || explicit_free_list_counter == 0)
    {
        assert(explicit_free_list_head == 0);
        assert(explicit_free_list_counter == 0);
        return;
    }

    if (explicit_free_list_counter == 1)
    {
        assert(get_prevfree(explicit_free_list_head) == explicit_free_list_head);
        assert(get_nextfree(explicit_free_list_head) == explicit_free_list_head);

        explicit_free_list_head = 0;
        explicit_free_list_counter = 0;

        return;
    }

    // counter >= 2
    uint64_t prev = get_prevfree(block);
    uint64_t next = get_nextfree(block);

    if (block == explicit_free_list_head)
    {
        explicit_free_list_head = next;
    }
    
    set_nextfree(prev, next);
    set_prevfree(next, prev);

    explicit_free_list_counter -= 1;
}

static int explicit_free_list_heap_init()
{
    if (implicit_free_list_heap_init() == 0)
    {
        return 0;
    }

    uint64_t first_block = get_firstblock();
    set_prevfree(first_block, first_block);
    set_nextfree(first_block, first_block);

    explicit_free_list_head = first_block;
    explicit_free_list_counter = 1;

    return 1;
}

static uint64_t explicit_free_list_mem_alloc(uint32_t size)
{
#ifdef DEBUG_MALLOC
    sprintf(debug_message, "mem_malloc(%u)", size);
#endif

    assert(0 < size && size < HEAP_MAX_SIZE - 4 - 8 - 4);

    uint64_t payload_vaddr = 0;
    
    uint32_t request_blocksize = round_up(size, 8) + 4 + 4;
    request_blocksize = request_blocksize < MIN_EXPLICIT_FREE_LIST_BLOCKSIZE ?
        MIN_EXPLICIT_FREE_LIST_BLOCKSIZE : request_blocksize;

    uint64_t b = explicit_free_list_head;

    // not thread safe
    uint32_t counter_copy = explicit_free_list_counter;
    for (int i = 0; i < counter_copy; ++ i)
    {
        uint32_t b_old_blocksize = get_blocksize(b);
        payload_vaddr = try_alloc_with_splitting(b, request_blocksize, MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

        if (payload_vaddr != 0)
        {
            uint32_t b_new_blocksize = get_blocksize(b);
            assert(b_new_blocksize <= b_old_blocksize);
            explicit_free_list_delete(b);

            if (b_old_blocksize > b_new_blocksize)
            {
                // b has been splitted
                uint64_t a = get_nextheader(b);
                assert(get_allocated(a) == 0);
                assert(get_blocksize(a) == b_old_blocksize - b_new_blocksize);
                explicit_free_list_insert(a);
            }

#ifdef DEBUG_MALLOC
            check_heap_correctness();
#endif
            return payload_vaddr;
        }
        else
        {
            // go to next block
            b = get_nextfree(b);
        }
    }

    // when no enough free block for current heap
    // request a new free physical & virtual page from OS
    uint64_t old_last = get_lastblock();
    if (get_allocated(old_last) == 0)
    {
        explicit_free_list_delete(old_last);
    }

    payload_vaddr = brk_when_almost_full(request_blocksize, MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

    uint64_t new_last = get_lastblock();
    if (get_allocated(new_last) == 0)
    {
        explicit_free_list_delete(new_last);
    }

    return payload_vaddr;
}

static void explicit_free_list_mem_free(uint64_t payload_addr)
{
    // TODO
}

/* ------------------------------------- */
/*  Tree-Managed Free Blocks             */
/* ------------------------------------- */

/*  Allocated block:

    ff ff ff f9/f1  [8n + 24] - footer
    ?? ?? ?? ??     [8n + 20] - padding
    xx xx ?? ??     [8n + 16] - payload & padding
    xx xx xx xx     [8n + 12] - payload
    xx xx xx xx     [8n + 8] - payload
    hh hh hh h9/h1  [8n + 4] - header

    Free block:

    ff ff ff f8/f0  [8n + 24] - footer
    ?? ?? ?? ??     [8n + 20]
    rr rr rr rr     [8n + 16] - right child bst node address
    ll ll ll ll     [8n + 12] - left child bst node address
    pp pp pp pp     [8n + 8] - previous bst node address
    hh hh hh h8/h0  [8n + 4] - header
*/

#define MIN_BST_BLOCKSIZE (20)

static uint32_t get_bst_prev(uint64_t header_vaddr)
{
    assert(get_firstblock() <= header_vaddr && header_vaddr <= get_lastblock());
    assert(header_vaddr % 8 == 4);
    assert(get_blocksize(header_vaddr) >= MIN_BST_BLOCKSIZE);

    return *(uint32_t *)&heap[header_vaddr + 4];
}

static uint32_t get_bst_left(uint64_t header_vaddr)
{
    assert(get_firstblock() <= header_vaddr && header_vaddr <= get_lastblock());
    assert(header_vaddr % 8 == 4);
    assert(get_blocksize(header_vaddr) >= MIN_BST_BLOCKSIZE);
    
    return *(uint32_t *)&heap[header_vaddr + 8];
}

static uint32_t get_bst_right(uint64_t header_vaddr)
{
    assert(get_firstblock() <= header_vaddr && header_vaddr <= get_lastblock());
    assert(header_vaddr % 8 == 4);
    assert(get_blocksize(header_vaddr) >= MIN_BST_BLOCKSIZE);
    
    return *(uint32_t *)&heap[header_vaddr + 12];
}

static uint64_t bst_root_node = 0;

static int bst_heap_init()
{
    // TODO
    return 0;
}

static uint64_t bst_mem_alloc(uint32_t size)
{
    // TODO
    return 0;
}

static void bst_mem_free(uint64_t payload_addr)
{
    // TODO
}

// #ifdef DEBUG_MALLOC

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

    for (int i = get_prologue(); i <= get_epilogue(); i += 4)
    {
        *(uint32_t *)&heap[i] = 0x1234abc0;
        assert(get_blocksize(i) == 0x1234abc0);
        assert(get_allocated(i) == 0);

        *(uint32_t *)&heap[i] = 0x1234abc1;
        assert(get_blocksize(i) == 0x1234abc0);
        assert(get_allocated(i) == 1);

        *(uint32_t *)&heap[i] = 0x1234abc8;
        assert(get_blocksize(i) == 0x1234abc8);
        assert(get_allocated(i) == 0);

        *(uint32_t *)&heap[i] = 0x1234abc9;
        assert(get_blocksize(i) == 0x1234abc8);
        assert(get_allocated(i) == 1);
    }

    printf("\033[32;1m\tPass\033[0m\n");
}

static void test_set_blocksize_allocated()
{
    printf("Testing setting block size to header ...\n");

    for (int i = get_prologue(); i <= get_epilogue(); i += 4)
    {
        set_blocksize(i, 0x1234abc0);
        set_allocated(i, 0);
        assert(get_blocksize(i) == 0x1234abc0);
        assert(get_allocated(i) == 0);

        set_blocksize(i, 0x1234abc0);
        set_allocated(i, 1);
        assert(get_blocksize(i) == 0x1234abc0);
        assert(get_allocated(i) == 1);

        set_blocksize(i, 0x1234abc8);
        set_allocated(i, 0);
        assert(get_blocksize(i) == 0x1234abc8);
        assert(get_allocated(i) == 0);

        set_blocksize(i, 0x1234abc8);
        set_allocated(i, 1);
        assert(get_blocksize(i) == 0x1234abc8);
        assert(get_allocated(i) == 1);
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
    uint64_t f = 0;

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

        if (allocated == 1 && (rand() % 3) >= 1)
        {
            // with previous allocated, 2/3 possibilities to be free
            allocated = 0;
        }
        else
        {
            allocated = 1;
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
    while (h != 0 && h < get_epilogue())
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
    while (h != 0 && get_firstblock() <= h)
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
            uint64_t p = mem_alloc(size);

            if (p != 0)
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
    assert(get_allocated(get_firstblock()) == 0);
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

// #endif