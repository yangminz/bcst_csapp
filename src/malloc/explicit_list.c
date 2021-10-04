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

// block8.c
// Manage small blocks
void block8_list_init();
void block8_list_insert(uint64_t free_header);
void block8_list_delete(uint64_t free_header);
linkedlist_internal_t block8_list;

#define MIN_EXPLICIT_FREE_LIST_BLOCKSIZE (16)

/* ------------------------------------- */
/*  Operations for List Block Structure  */
/* ------------------------------------- */

static int compare_nodes(uint64_t first, uint64_t second)
{
    return !(first == second);
}

static int is_null_node(uint64_t node_id)
{
    return node_id == NULL_ID;
}

static uint64_t get_prevfree(uint64_t header_vaddr)
{
    return get_field32_block_ptr(header_vaddr, MIN_EXPLICIT_FREE_LIST_BLOCKSIZE, 4);
}

static uint64_t get_nextfree(uint64_t header_vaddr)
{
    return get_field32_block_ptr(header_vaddr, MIN_EXPLICIT_FREE_LIST_BLOCKSIZE, 8);
}

static int set_prevfree(uint64_t header_vaddr, uint64_t prev_vaddr)
{
    set_field32_block_ptr(header_vaddr, prev_vaddr, MIN_EXPLICIT_FREE_LIST_BLOCKSIZE, 4);
    return 1;
}

static int set_nextfree(uint64_t header_vaddr, uint64_t next_vaddr)
{
    set_field32_block_ptr(header_vaddr, next_vaddr, MIN_EXPLICIT_FREE_LIST_BLOCKSIZE, 8);
    return 1;
}

// register the 5 functions above to be called by the linked list framework
static linkedlist_node_interface i_free_block =
{
    .compare_nodes = &compare_nodes,
    .is_null_node = &is_null_node,
    .get_node_prev = &get_prevfree,
    .set_node_prev = &set_prevfree,
    .get_node_next = &get_nextfree,
    .set_node_next = &set_nextfree,
};

/* ------------------------------------- */
/*  Operations for Linked List           */
/* ------------------------------------- */

static int update_head(linkedlist_internal_t *this, uint64_t block_vaddr)
{
    if (this == NULL)
    {
        return 0;
    }
    
    assert(block_vaddr == NULL_ID || (get_firstblock() <= block_vaddr && block_vaddr <= get_lastblock()));
    assert(block_vaddr == NULL_ID || block_vaddr % 8 == 4);
    assert(block_vaddr == NULL_ID || get_blocksize(block_vaddr) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

    this->head = block_vaddr;
    return 1;
}

// The explicit free linked list
static linkedlist_internal_t explicit_list;

static void explist_list_init()
{
    explicit_list.head = NULL_ID;
    explicit_list.count = 0;
    explicit_list.update_head = &update_head;
}

static void explicit_list_insert(uint64_t free_header)
{
    assert(get_firstblock() <= free_header && free_header <= get_lastblock());
    assert(free_header % 8 == 4);
    assert(get_blocksize(free_header) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);
    assert(get_allocated(free_header) == FREE);

    linkedlist_internal_insert(&explicit_list, &i_free_block, free_header);
}

static void explicit_list_delete(uint64_t free_header)
{
    assert(get_firstblock() <= free_header && free_header <= get_lastblock());
    assert(free_header % 8 == 4);
    assert(get_blocksize(free_header) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);
    // assert(get_allocated(free_header) == FREE);

    linkedlist_internal_delete(&explicit_list, &i_free_block, free_header);
    set_prevfree(free_header, NIL);
    set_nextfree(free_header, NIL);
}

/* ------------------------------------- */
/*  For Debugging                        */
/* ------------------------------------- */

static void explicit_list_print()
{
    uint64_t p = explicit_list.head;
    printf("explicit free list <{%lu},{%lu}>:\n", explicit_list.head, explicit_list.count);
    for (int i = 0; i < explicit_list.count; ++ i)
    {
        printf("<%lu:%u/%u> ", p, get_blocksize(p), get_allocated(p));
        p = get_nextfree(p);
    }
    printf("\n");
}

/* ------------------------------------- */
/*  Implementation                       */
/* ------------------------------------- */

#ifdef EXPLICIT_FREE_LIST

int initialize_free_block()
{
    uint64_t first_header = get_firstblock();
    
    set_prevfree(first_header, first_header);
    set_nextfree(first_header, first_header);

    explist_list_init();
    explicit_list_insert(first_header);

    // init small block list
    block8_list_init();

    return 1;
}

uint64_t search_free_block(uint32_t payload_size, uint32_t *alloc_blocksize)
{
    // search 8-byte block list
    if (payload_size <= 4 && block8_list.count != 0)
    {
        // a small block and 8-byte list is not empty
        *alloc_blocksize = 8;
        return block8_list.head;
    }
    
    uint32_t free_blocksize = round_up(payload_size, 8) + 4 + 4;
    free_blocksize = free_blocksize < MIN_EXPLICIT_FREE_LIST_BLOCKSIZE ?
        MIN_EXPLICIT_FREE_LIST_BLOCKSIZE : free_blocksize;
    *alloc_blocksize = free_blocksize;

    // search explicit free list
    uint64_t b = explicit_list.head;
    uint32_t counter_copy = explicit_list.count;
    for (int i = 0; i < counter_copy; ++ i)
    {
        uint32_t b_blocksize = get_blocksize(b);
        uint32_t b_allocated = get_allocated(b);

        if (b_allocated == FREE && free_blocksize <= b_blocksize)
        {
            return b;
        }
        else
        {
            b = get_nextfree(b);
        }
    }

    return NIL;
}

int insert_free_block(uint64_t free_header)
{
    assert(free_header % 8 == 4);
    assert(get_firstblock() <= free_header && free_header <= get_lastblock());
    assert(get_allocated(free_header) == FREE);

    uint32_t blocksize = get_blocksize(free_header);
    assert(blocksize % 8 == 0);
    assert(blocksize >= 8);

    switch (blocksize)
    {
        case 8:
            block8_list_insert(free_header);
            break;
        
        default:
            explicit_list_insert(free_header);
            break;
    }

    return 1;
}

int delete_free_block(uint64_t free_header)
{
    assert(free_header % 8 == 4);
    assert(get_firstblock() <= free_header && free_header <= get_lastblock());
    assert(get_allocated(free_header) == FREE);

    uint32_t blocksize = get_blocksize(free_header);
    assert(blocksize % 8 == 0);
    assert(blocksize >= 8);

    switch (blocksize)
    {
        case 8:
            block8_list_delete(free_header);
            break;
        
        default:
            explicit_list_delete(free_header);
            break;
    }

    return 1;
}

void check_freeblock_correctness()
{
    uint32_t explicit_list_counter = 0;
    uint64_t b = get_firstblock(); 
    int head_exists = 0;
    while(b <= get_lastblock())
    {
        if (get_allocated(b) == FREE && get_blocksize(b) > 8)
        {
            uint64_t prev = get_prevfree(b);
            uint64_t next = get_nextfree(b);

            assert(get_allocated(prev) == FREE);
            assert(get_allocated(next) == FREE);
            assert(get_nextfree(prev) == b);
            assert(get_prevfree(next) == b);

            if (b == explicit_list.head)
            {
                head_exists = 1;
            }

            explicit_list_counter += 1;
        }
        b = get_nextheader(b);
    }
    assert(head_exists == 1);
    assert(explicit_list_counter == explicit_list.count);

    uint64_t p = explicit_list.head;
    uint64_t n = explicit_list.head;
    for (int i = 0; i < explicit_list.count; ++ i)
    {
        assert(get_allocated(p) == FREE);
        assert(get_blocksize(p) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

        assert(get_allocated(n) == FREE);
        assert(get_blocksize(n) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);

        p = get_prevfree(p);
        n = get_nextfree(n);
    }
    assert(p == explicit_list.head);
    assert(n == explicit_list.head);
}
#endif