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
#include "headers/algorithm.h"

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

static uint64_t get_prevfree_block8(uint64_t header_vaddr)
{
    assert(header_vaddr % 8 == 4);
    assert(get_allocated(header_vaddr) == FREE);
    // header is 8X + 4
    uint32_t value = *(uint32_t *)&heap[header_vaddr];
    return 4 + (value & 0xFFFFFFF8);
}

static uint64_t get_nextfree_block8(uint64_t header_vaddr)
{
    assert(header_vaddr % 8 == 4);
    assert(get_allocated(header_vaddr) == FREE);
    // header is 8X + 4
    uint32_t value = *(uint32_t *)&heap[header_vaddr + 4];
    return 4 + (value & 0xFFFFFFF8);
}

static int set_prevfree_block8(uint64_t header_vaddr, uint64_t prev_vaddr)
{
    // we set by header only
    assert(header_vaddr % 8 == 4);
    assert(get_allocated(header_vaddr) == FREE);
    assert(prev_vaddr % 8 == 4);
    *(uint32_t *)&heap[header_vaddr] &= 0x00000007; // reset prev pointer
    *(uint32_t *)&heap[header_vaddr] |= (prev_vaddr & 0xFFFFFFF8);
    return 1;
}

static int set_nextfree_block8(uint64_t header_vaddr, uint64_t next_vaddr)
{
    // we set by header only
    assert(header_vaddr % 8 == 4);
    assert(get_allocated(header_vaddr) == FREE);
    assert(next_vaddr % 8 == 4);
    header_vaddr += 4;
    *(uint32_t *)&heap[header_vaddr] &= 0x00000007; // reset next pointer
    *(uint32_t *)&heap[header_vaddr] |= (next_vaddr & 0xFFFFFFF8);
    return 1;
}

// register the 5 functions above to be called by the linked list framework
static linkedlist_node_interface i_block8 =
{
    .compare_nodes = &compare_nodes,
    .is_null_node = &is_null_node,
    .get_node_prev = &get_prevfree_block8,
    .set_node_prev = &set_prevfree_block8,
    .get_node_next = &get_nextfree_block8,
    .set_node_next = &set_nextfree_block8,
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
    assert(block_vaddr == NULL_ID || get_blocksize(block_vaddr) == 8);

    this->head = block_vaddr;
    return 1;
}

linkedlist_internal_t small_list;

void small_list_init()
{
    small_list.head = NULL_ID;
    small_list.count = 0;
    small_list.update_head = &update_head;
}

void small_list_insert(uint64_t free_header)
{
    assert(get_firstblock() <= free_header && free_header <= get_lastblock());
    assert(free_header % 8 == 4);
    assert(get_blocksize(free_header) == 8);
    assert(get_allocated(free_header) == FREE);

    linkedlist_internal_insert(&small_list, &i_block8, free_header);
}

void small_list_delete(uint64_t free_header)
{
    assert(get_firstblock() <= free_header && free_header <= get_lastblock());
    assert(free_header % 8 == 4);
    assert(get_blocksize(free_header) == 8);

    linkedlist_internal_delete(&small_list, &i_block8, free_header);
}

// from segregated list
extern void check_size_list_correctness(
    linkedlist_internal_t *list, linkedlist_node_interface *i_node, 
    uint32_t min_size, uint32_t max_size);

void small_list_check_free_blocks()
{
    check_size_list_correctness(&small_list, &i_block8, 8, 8);
}