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
void small_list_init();
void small_list_insert(uint64_t free_header);
void small_list_delete(uint64_t free_header);
linkedlist_internal_t small_list;
void small_list_check_free_blocks();

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

uint64_t get_nextfree(uint64_t header_vaddr)
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
linkedlist_internal_t explicit_list;

void explist_list_init()
{
    explicit_list.head = NULL_ID;
    explicit_list.count = 0;
    explicit_list.update_head = &update_head;
}

void explicit_list_insert(uint64_t free_header)
{
    assert(get_firstblock() <= free_header && free_header <= get_lastblock());
    assert(free_header % 8 == 4);
    assert(get_blocksize(free_header) >= MIN_EXPLICIT_FREE_LIST_BLOCKSIZE);
    assert(get_allocated(free_header) == FREE);

    linkedlist_internal_insert(&explicit_list, &i_free_block, free_header);
}

void explicit_list_delete(uint64_t free_header)
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

// from segregated list
extern void check_size_list_correctness(
    linkedlist_internal_t *list, linkedlist_node_interface *i_node, 
    uint32_t min_size, uint32_t max_size);

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

void check_block16_correctness()
{
    check_size_list_correctness(&explicit_list, &i_free_block, 16, 16);
}

/* ------------------------------------- */
/*  Implementation                       */
/* ------------------------------------- */

int explicit_list_initialize_free_block()
{
    uint64_t first_header = get_firstblock();
    
    set_prevfree(first_header, first_header);
    set_nextfree(first_header, first_header);

    explist_list_init();
    explicit_list_insert(first_header);

    // init small block list
    small_list_init();

    return 1;
}

uint64_t explicit_list_search_free_block(uint32_t payload_size, uint32_t *alloc_blocksize)
{
    // search 8-byte block list
    if (payload_size <= 4 && small_list.count != 0)
    {
        // a small block and 8-byte list is not empty
        *alloc_blocksize = 8;
        return small_list.head;
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

int explicit_list_insert_free_block(uint64_t free_header)
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
            small_list_insert(free_header);
            break;
        
        default:
            explicit_list_insert(free_header);
            break;
    }

    return 1;
}

int explicit_list_delete_free_block(uint64_t free_header)
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
            small_list_delete(free_header);
            break;
        
        default:
            explicit_list_delete(free_header);
            break;
    }

    return 1;
}

void explicit_list_check_free_block()
{
    small_list_check_free_blocks();
    check_size_list_correctness(&explicit_list, &i_free_block, 16, 0xFFFFFFFF);
}