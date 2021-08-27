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

int heap_init();
uint64_t mem_alloc(uint32_t size);
void mem_free(uint64_t vaddr);

uint64_t heap_start_vaddr = 0;
uint64_t heap_end_vaddr = 0;

#define HEAP_MAX_SIZE (4096 * 8)
uint8_t heap[HEAP_MAX_SIZE];

// Round up to next multiple of n:
// if (x == k * n)
// return x
// else, x = k * n + m and m < n
// return (k + 1) * n
static uint64_t round_up(uint64_t x, uint64_t n)
{
    return n * ((x + n - 1) / n);
}

// applicapable for both header & footer
static uint32_t get_blocksize(uint64_t header_vaddr)
{
    assert(heap_start_vaddr <= header_vaddr && header_vaddr <= heap_end_vaddr);
    assert(header_vaddr & 0x3 == 0x0);  // header & footer should be 4 bytes alignment

    uint32_t header_value = *(uint32_t *)&heap[header_vaddr];
    return header_value & 0xFFFFFFF8;
}

// applicapable for both header & footer
static void set_blocksize(uint64_t header_vaddr, uint32_t blocksize)
{
    assert(heap_start_vaddr <= header_vaddr && header_vaddr <= heap_end_vaddr);
    assert(header_vaddr & 0x3 == 0x0);  // header & footer should be 4 bytes alignment
    assert(blocksize & 0x7 == 0x0); // blocksize should be 8 bytes aligned

    *(uint32_t *)&heap[header_vaddr] &= 0x00000007;
    *(uint32_t *)&heap[header_vaddr] |= blocksize;
}

// applicapable for both header & footer
static uint32_t get_allocated(uint64_t header_vaddr)
{
    assert(heap_start_vaddr <= header_vaddr && header_vaddr <= heap_end_vaddr);
    assert(header_vaddr & 0x3 == 0x0);  // header & footer should be 4 bytes alignment

    uint32_t header_value = *(uint32_t *)&heap[header_vaddr];
    return header_value & 0x1;    
}

// applicapable for both header & footer
static void set_allocated(uint64_t header_vaddr, uint32_t allocated)
{
    assert(heap_start_vaddr <= header_vaddr && header_vaddr <= heap_end_vaddr);
    assert(header_vaddr & 0x3 == 0x0);  // header & footer should be 4 bytes alignment

    *(uint32_t *)&heap[header_vaddr] &= 0xFFFFFFF8;
    *(uint32_t *)&heap[header_vaddr] |= (allocated & 0x1);
}

static uint64_t get_payload(uint64_t vaddr)
{
    // vaddr can be:
    // 1. starting address of the block (8 * n + 4)
    // 2. starting address of the payload (8 * m)
    return round_up(vaddr, 8);
}

static uint64_t get_blockheader(uint64_t vaddr)
{
    // vaddr can be:
    // 1. starting address of the block (8 * n + 4)
    // 2. starting address of the payload (8 * m)
    return round_up(vaddr, 8) - 4;
}

static uint64_t get_nextheader(uint64_t vaddr)
{
    // vaddr can be:
    // 1. starting address of the block (8 * n + 4)
    // 2. starting address of the payload (8 * m)
    uint64_t header_vaddr = get_blockheader(vaddr);
    uint32_t block_size = get_blocksize(header_vaddr);

    uint64_t next_header_vaddr = header_vaddr + block_size;
    assert(heap_start_vaddr <= next_header_vaddr &&
        next_header_vaddr <= heap_end_vaddr);
    
    return next_header_vaddr;
}

static uint64_t get_prevheader(uint64_t vaddr)
{
    // vaddr can be:
    // 1. starting address of the block (8 * n + 4)
    // 2. starting address of the payload (8 * m)
    uint64_t header_vaddr = get_blockheader(vaddr);
    assert(header_vaddr >= 16);

    uint64_t prev_footer_vaddr = header_vaddr - 4;
    uint32_t prev_blocksize = get_blocksize(prev_footer_vaddr);

    uint64_t prev_header_vaddr = header_vaddr - prev_blocksize;
    assert(heap_start_vaddr <= prev_header_vaddr &&
        prev_header_vaddr <= heap_end_vaddr - 12);
    
    return prev_header_vaddr;
}

static int heap_check()
{
    // rule 1: block[0] ==> A/F
    // rule 2: block[-1] ==> A/F
    // rule 3: block[i] == A ==> block[i-1] == A/F && block[i+1] == A/F
    // rule 4: block[i] == F ==> block[i-1] == A && block[i+1] == A
    // these 4 rules ensures that
    // adjacent free blocks are always merged together
    // henceforth external fragmentation are minimized
    return 0;
}

int heap_init()
{
    // heap_start_vaddr is the starting address of the first block
    // the payload of the first block is 8B aligned ([8])
    // so the header address of the first block is [8] - 4 = [4]
    heap_start_vaddr = 4;

    set_blocksize(heap_start_vaddr, 4096 - 8);
    set_allocated(heap_start_vaddr, 0);

    // we do not set footer for the last block in heap
    heap_end_vaddr = 4096 - 1;

    return 0;
}

// size - requested payload size
// return - the virtual address of payload
uint64_t mem_alloc(uint32_t size)
{
    uint32_t request_blocksize = round_up(size, 8) + 4 + 4;

    uint64_t b = heap_start_vaddr;
    while (b <= heap_end_vaddr)
    {
        uint32_t b_blocksize = get_blocksize(b);
        uint32_t b_allocated = get_allocated(b);

        if (b_allocated == 0 && b_blocksize >= request_blocksize)
        {
            // allocate this block
            if (b_blocksize > request_blocksize)
            {
                // split this block `b`
                // b_blocksize - request_blocksize >= 8
                set_allocated(b, 1);
                set_blocksize(b, request_blocksize);

                // set the left splitted block
                // in the extreme situation, next block size == 8
                // which makes the whole block of next to be:
                // [0x00000008, 0x00000008]
                uint64_t next_header_vaddr = b + request_blocksize;
                set_allocated(next_header_vaddr, 0);
                set_blocksize(next_header_vaddr, b_blocksize - request_blocksize);

                return get_payload(b);
            }
            else
            {
                // no need to split this block
                // set_blocksize(b, request_blocksize);
                set_allocated(b, 1);
                return get_payload(b);
            }
        }
        else
        {
            // go to next block
            b = get_nextheader(b);
        }
    }

    // <==> return NULL;
    return 0;
}

void mem_free(uint64_t vaddr)
{
    assert(heap_start_vaddr <= vaddr && vaddr <= heap_end_vaddr);
    assert(vaddr & 0x7 == 0x0);

    uint64_t req = get_blockheader(vaddr);
    uint32_t req_allocated = get_allocated(req);
    uint32_t req_blocksize = get_blocksize(req);
    assert(req_allocated == 1);

    // block starting address of next & prev blocks
    // TODO: corner case - req can be the first or last block
    uint64_t next = get_nextheader(vaddr);
    uint64_t prev = get_prevheader(vaddr);

    uint32_t next_allocated = get_allocated(next);
    uint32_t prev_allocated = get_allocated(prev);

    uint32_t next_blocksize = get_blocksize(next);
    uint32_t prev_blocksize = get_blocksize(prev);

    if (next_allocated == 1 && prev_allocated == 1)
    {
        // case 1: *A(A->F)A*
        // ==> *AFA*
        set_allocated(req, 0);
    }
    else if (next_allocated == 0 && prev_allocated == 1)
    {
        // case 2: *A(A->F)FA
        // ==> *AFFA ==> *A[FF]A merge current and next
        set_allocated(req, 0);
        set_blocksize(req, req_blocksize + next_blocksize);
    }
    else if (next_allocated == 1 && prev_allocated == 0)
    {
        // case 3: AF(A->F)A*
        // ==> AFFA* ==> A[FF]A* merge current and prev
        set_allocated(prev, 0);
        set_blocksize(prev, req_blocksize + prev_blocksize);
    }
    else
    {
        // case 4: AF(A->F)FA
        // ==> AFFFA ==> A[FFF]A merge current and prev and next
        set_allocated(prev, 0);
        set_blocksize(prev, req_blocksize + prev_blocksize + next_blocksize);
    }
}

#ifdef DEBUG_MALLOC
int main()
{
    printf("malloc!\n");

    heap_init();

    return 0;
}
#endif