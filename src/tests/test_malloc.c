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
#include "headers/allocator.h"
#include "headers/algorithm.h"
#include "headers/color.h"

void check_heap_correctness();

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

    printf(GREENSTR("Pass\n"));
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

    for (int i = get_prologue() + 16; i <= get_epilogue(); i += 4)
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

    printf(GREENSTR("Pass\n"));
}

static void test_set_blocksize_allocated()
{
    printf("Testing setting block size to header ...\n");

    heap_init();

    for (int i = get_firstblock() + 16; i < get_epilogue(); i += 4)
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

    printf(GREENSTR("Pass\n"));
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

    printf(GREENSTR("Pass\n"));
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

        set_allocated(h, allocated);
        set_blocksize(h, blocksize);

        f = h + blocksize - 4;
        set_allocated(f, allocated);
        set_blocksize(f, blocksize);

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

    printf(GREENSTR("Pass\n"));
}

// collection for the pointers
linkedlist_t *ptrs;

static void print_pointers()
{
    for (int i = 0; i < ptrs->count; ++ i)
    {
        linkedlist_node_t *p = linkedlist_next(ptrs);
        printf("%lu ", p->value);
    }
    printf("\n");
}

static void test_malloc_free()
{
    printf("Testing malloc & free ...\n");

    heap_init();
    check_heap_correctness();

    srand(123456);
    ptrs = linkedlist_construct();
    
    for (int i = 0; i < 50000; ++ i)
    {
        if ((rand() & 0x1) == 0)
        {
            // malloc
            uint32_t size = rand() % 1024 + 1;  // a non zero value
            uint64_t p = mem_alloc(size);

            if (p != NIL)
            {
                linkedlist_add(ptrs, p);
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
        linkedlist_delete(ptrs, t);
    }
    assert(ptrs->count == 0);
    linkedlist_free(ptrs);

    // finally there should be only one free block
    assert(is_lastblock(get_firstblock()) == 1);
    assert(get_allocated(get_firstblock()) == FREE);
    check_heap_correctness();

    printf(GREENSTR("Pass\n"));
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