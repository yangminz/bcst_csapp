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

#define AF_BIT (0)
#define P8_BIT (1)
#define B8_BIT (2)

static void set_bit(uint64_t vaddr, int bit_offset)
{
    uint32_t vector = 1 << bit_offset;

    assert((vaddr & 0x3) == 0);
    assert(get_prologue() <= vaddr && vaddr <= get_epilogue());

    *(uint32_t *)&heap[vaddr] |= vector;
}

static void reset_bit(uint64_t vaddr, int bit_offset)
{
    uint32_t vector = 1 << bit_offset;

    assert((vaddr & 0x3) == 0);
    assert(get_prologue() <= vaddr && vaddr <= get_epilogue());

    *(uint32_t *)&heap[vaddr] &= (~vector);
}

static int is_bit_set(uint64_t vaddr, int bit_offset)
{
    assert((vaddr & 0x3) == 0);
    assert(get_prologue() <= vaddr && vaddr <= get_epilogue());

    return (*(uint32_t *)&heap[vaddr] >> bit_offset) & 1;
}

// applicapable for both header & footer
uint32_t get_blocksize(uint64_t header_vaddr)
{
    if (header_vaddr == NIL)
    {
        return 0;
    }

    assert(get_prologue() <= header_vaddr && header_vaddr <= get_epilogue());
    assert((header_vaddr & 0x3) == 0x0);  // header & footer should be 4 bytes alignment

    if (is_bit_set(header_vaddr, B8_BIT) == 1)
    {
        if (get_allocated(header_vaddr) == ALLOCATED)
        {
            assert((*(uint32_t *)&heap[header_vaddr] & 0xFFFFFFF8) == 8);
        }
        return 8;
    }
    else
    {
        // B8 is unset - an ordinary block
        return (*(uint32_t *)&heap[header_vaddr] & 0xFFFFFFF8);
    }
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

    uint64_t next_header_vaddr = header_vaddr + blocksize;
    if (blocksize == 8)
    {
        // small block is special
        if (header_vaddr % 8 == 0)
        {
            // do not set footer of small block
            // reset to header
            header_vaddr = header_vaddr - 4;
        }

        set_bit(header_vaddr, B8_BIT);
        if (next_header_vaddr <= get_epilogue())
        {
            set_bit(next_header_vaddr, P8_BIT);
        }

        if (get_allocated(header_vaddr) == FREE)
        {
            // free 8-byte does not set block size
            return;
        }
        // else, set header blocksize 8
    }
    else
    {
        reset_bit(header_vaddr, B8_BIT);
        if (next_header_vaddr <= get_epilogue())
        {
            reset_bit(next_header_vaddr, P8_BIT);
        }
    }

    *(uint32_t *)&heap[header_vaddr] &= 0x00000007; // reset size
    *(uint32_t *)&heap[header_vaddr] |= blocksize;  // set size
}

// applicapable for both header & footer for ordinary blocks
// header only for small block 8-Byte
uint32_t get_allocated(uint64_t header_vaddr)
{
    if (header_vaddr == NIL)
    {
        // NULL can be considered as allocated
        return ALLOCATED;
    }

    assert(get_prologue() <= header_vaddr && header_vaddr <= get_epilogue());
    assert((header_vaddr & 0x3) == 0x0);

    if (header_vaddr %8 == 0)
    {
        // footer
        // check if 8-byte small block
        uint64_t next_header_vaddr = header_vaddr - 4;
        if (next_header_vaddr <= get_epilogue())
        {
            // check P8 bit of next
            if (is_bit_set(next_header_vaddr, P8_BIT) == 1)
            {
                // current block is 8-byte, no footer. check header instead
                header_vaddr -= 4;
            }
            // else, current block has footer
        }
        else
        {
            // this is block is epilogue but it's 8X
            assert(0);
        }
    }

    // header
    return (*(uint32_t *)&heap[header_vaddr] & 0x1);
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

    if (header_vaddr %8 == 0)
    {
        // footer
        // check if 8-byte small block
        uint64_t next_header_vaddr = header_vaddr - 4;
        if (next_header_vaddr <= get_epilogue())
        {
            // check P8 bit of next
            if (is_bit_set(next_header_vaddr, P8_BIT) == 1)
            {
                // current block is 8-byte, no footer. check header instead
                header_vaddr -= 4;
            }
            // else, current block has footer
        }
        else
        {
            // this is block is epilogue but it's 8X
            assert(0);
        }
    }

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
    assert(get_prologue() <= vaddr && vaddr <= get_epilogue());

    // vaddr can be:
    // 1. starting address of the block (8 * n + 4)
    // 2. starting address of the payload (8 * n + 8)
    assert((vaddr & 0x3) == 0);
    
    return round_up(vaddr, 8) - 4;
}

uint64_t get_footer(uint64_t vaddr)
{
    if (vaddr == NIL)
    {
        return NIL;
    }    
    assert(get_prologue() <= vaddr && vaddr < get_epilogue());

    // vaddr can be:
    // 1. starting address of the block (8 * n + 4)
    // 2. starting address of the payload (8 * m)
    assert(vaddr % 8 == 4);

    uint64_t header_vaddr = get_header(vaddr);
    uint64_t footer_vaddr = header_vaddr + get_blocksize(header_vaddr) - 4;

    assert(get_firstblock() < footer_vaddr && footer_vaddr < get_epilogue());
    assert(footer_vaddr % 8 == 0);
    return footer_vaddr;
}

/* ------------------------------------- */
/*  Heap Operations                      */
/* ------------------------------------- */

uint64_t get_nextheader(uint64_t vaddr)
{
    if (vaddr == NIL || vaddr == get_epilogue())
    {
        return NIL;
    }

    assert(get_prologue() <= vaddr && vaddr < get_epilogue());

    // vaddr can be:
    // 1. starting address of the block (8 * n + 4)
    // 2. starting address of the payload (8 * m)
    uint64_t header_vaddr = get_header(vaddr);
    uint32_t block_size = get_blocksize(header_vaddr);

    uint64_t next_header_vaddr = header_vaddr + block_size;
    assert(get_firstblock() < next_header_vaddr &&
        next_header_vaddr <= get_epilogue());
    
    return next_header_vaddr;
}

uint64_t get_prevheader(uint64_t vaddr)
{
    if (vaddr == NIL || vaddr == get_prologue())
    {
        return NIL;
    }
    
    assert(get_firstblock() <= vaddr && vaddr <= get_epilogue());

    uint64_t header_vaddr = get_header(vaddr);
    uint32_t header_value = *(uint32_t *)&heap[header_vaddr];
    uint64_t prev_header_vaddr;

    // check P8 bit 0010
    if (is_bit_set(header_vaddr, P8_BIT) == 1)
    {
        // previous block is 8-byte block
        prev_header_vaddr = header_vaddr - 8;
        assert(get_blocksize(prev_header_vaddr) == 8);
        
        // check B8 bit
        assert(is_bit_set(prev_header_vaddr, B8_BIT) == 1);

        return prev_header_vaddr;
    }
    else
    {
        // previous block is bigger than 8 bytes
        uint64_t prev_footer_vaddr = header_vaddr - 4;
        uint32_t prev_blocksize = get_blocksize(prev_footer_vaddr);

        prev_header_vaddr = header_vaddr - prev_blocksize;
        assert(get_prologue() <= prev_header_vaddr &&
            prev_header_vaddr < get_epilogue());
        assert(get_blocksize(prev_header_vaddr) == get_blocksize(prev_footer_vaddr));
        assert(get_allocated(prev_header_vaddr) == get_allocated(prev_footer_vaddr));
        
        return prev_header_vaddr;
    }
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
    return get_prevheader(epilogue_header);
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
    assert(block_ptr == NIL || block_ptr % 8 == 4);
    assert(block_ptr == NIL || get_blocksize(block_ptr) >= min_blocksize);

    assert(offset % 4 == 0);

    // actually a 32-bit pointer
    assert((block_ptr >> 32) == 0);
    *(uint32_t *)&heap[header_vaddr + offset] = (uint32_t)(block_ptr & 0xFFFFFFFF);
}