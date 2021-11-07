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

// Manage small blocks - 8 Bytes
void small_list_init();
void small_list_insert(uint64_t free_header);
void small_list_delete(uint64_t free_header);
linkedlist_internal_t small_list;
void small_list_check_free_blocks();

// Manage small blocks - 16 Bytes
void explist_list_init();
uint64_t explicit_list_search(uint64_t free_blocksize);
void explicit_list_insert(uint64_t free_header);
void explicit_list_delete(uint64_t free_header);
linkedlist_internal_t explicit_list;
uint64_t get_nextfree(uint64_t header_vaddr);
void check_block16_correctness();

void bst_internal_print(uint64_t node, rbtree_node_interface *i_node);

/* ------------------------------------- */
/*  Operations for Tree Block Structure  */
/* ------------------------------------- */

static int compare_nodes(uint64_t first, uint64_t second)
{
    return !(first == second);
}

static int is_null_node(uint64_t header_vaddr)
{
    if (get_firstblock() <= header_vaddr &&
        header_vaddr <= get_lastblock() &&
        header_vaddr % 8 == 4)
    {
        return 0;
    }
    return 1;
}

static uint64_t get_redblack_tree_parent(uint64_t header_vaddr)
{
    return get_field32_block_ptr(header_vaddr, 
        MIN_REDBLACK_TREE_BLOCKSIZE, 4);
}

static uint64_t get_redblack_tree_left(uint64_t header_vaddr)
{
    return get_field32_block_ptr(header_vaddr, 
        MIN_REDBLACK_TREE_BLOCKSIZE, 8);
}

static uint64_t get_redblack_tree_right(uint64_t header_vaddr)
{
    return get_field32_block_ptr(header_vaddr, 
        MIN_REDBLACK_TREE_BLOCKSIZE, 12);
}

static rb_color_t get_redblack_tree_color(uint64_t header_vaddr)
{
    if (header_vaddr == NIL)
    {
        // default BLACK
        return COLOR_BLACK;
    }

    assert(get_prologue() <= header_vaddr && 
        header_vaddr <= get_epilogue());
    assert(header_vaddr % 8 == 4);
    assert(get_blocksize(header_vaddr) >= MIN_REDBLACK_TREE_BLOCKSIZE);

    uint64_t footer_vaddr = get_footer(header_vaddr);
    uint32_t footer_value = *(uint32_t *)&heap[footer_vaddr];
    return (rb_color_t)((footer_value >> 1) & 0x1);
}

static uint64_t get_redblack_tree_key(uint64_t header_vaddr)
{
    uint32_t blocksize = get_blocksize(header_vaddr);
    return blocksize;
}

static int set_redblack_tree_parent(uint64_t header_vaddr,
    uint64_t prev_vaddr)
{
    set_field32_block_ptr(header_vaddr, prev_vaddr, 
        MIN_REDBLACK_TREE_BLOCKSIZE, 4);
    return 1;
}

static int set_redblack_tree_left(uint64_t header_vaddr, 
    uint64_t left_vaddr)
{
    set_field32_block_ptr(header_vaddr, left_vaddr, 
        MIN_REDBLACK_TREE_BLOCKSIZE, 8);
    return 1;
}

static int set_redblack_tree_right(uint64_t header_vaddr, 
    uint64_t right_vaddr)
{
    set_field32_block_ptr(header_vaddr, right_vaddr, 
        MIN_REDBLACK_TREE_BLOCKSIZE, 12);
    return 1;
}

static int set_redblack_tree_color(uint64_t header_vaddr, rb_color_t color)
{
    if (header_vaddr == NIL)
    {
        return 0;
    }

    assert(color == COLOR_BLACK || color == COLOR_RED);
    assert(get_prologue() <= header_vaddr && 
        header_vaddr <= get_epilogue());
    assert(header_vaddr % 8 == 4);
    assert(get_blocksize(header_vaddr) >= MIN_REDBLACK_TREE_BLOCKSIZE);

    uint64_t footer_vaddr = get_footer(header_vaddr);
    *(uint32_t *)&heap[footer_vaddr] &= 0xFFFFFFFD;
    *(uint32_t *)&heap[footer_vaddr] |= ((color & 0x1) << 1);

    return 1;
}

static int set_redblack_tree_key(uint64_t header_vaddr, uint64_t blocksize)
{
    assert((blocksize & 0xFFFFFFFF00000000) == 0);
    set_blocksize(header_vaddr, (uint32_t)blocksize);
    return 1;
}

static rbtree_node_interface i_node = 
{
    .construct_node = NULL,
    .destruct_node = NULL,
    .compare_nodes = &compare_nodes,
    .is_null_node = &is_null_node,
    .get_parent = &get_redblack_tree_parent,
    .set_parent = &set_redblack_tree_parent,
    .get_leftchild = &get_redblack_tree_left,
    .set_leftchild = &set_redblack_tree_left,
    .get_rightchild = &get_redblack_tree_right,
    .set_rightchild = &set_redblack_tree_right,
    .get_color = &get_redblack_tree_color,
    .set_color = &set_redblack_tree_color,
    .get_key = &get_redblack_tree_key,
    .set_key = &set_redblack_tree_key,
};

/* ------------------------------------- */
/*  Operations for Red-Black Tree        */
/* ------------------------------------- */

static int update_root(rbtree_internal_t *this, uint64_t block_vaddr)
{
    if (this == NULL)
    {
        return 0;
    }
    
    assert(block_vaddr == NULL_ID ||
        (get_firstblock() <= block_vaddr && block_vaddr <= get_lastblock()));
    assert(block_vaddr == NULL_ID || 
        block_vaddr % 8 == 4);
    assert(block_vaddr == NULL_ID || 
        get_blocksize(block_vaddr) >= MIN_REDBLACK_TREE_BLOCKSIZE);

    this->root = block_vaddr;
    return 1;
}

// The red-black tree
static rbtree_internal_t rbt;

static void redblack_tree_init()
{
    rbt.root = NULL_ID;
    rbt.update_root = &update_root;
}

static void redblack_tree_print()
{
    bst_internal_print(rbt.root, &i_node);
}

static void redblack_tree_insert(uint64_t node_ptr)
{
    set_redblack_tree_parent(node_ptr, NIL);
    set_redblack_tree_left(node_ptr, NIL);
    set_redblack_tree_right(node_ptr, NIL);
    set_redblack_tree_color(node_ptr, COLOR_RED);

    rbt_internal_insert(&rbt, &i_node, node_ptr);
}

static void redblack_tree_delete(uint64_t node_ptr)
{
    rbt_internal_delete(&rbt, &i_node, node_ptr);

    set_redblack_tree_parent(node_ptr, NIL);
    set_redblack_tree_left(node_ptr, NIL);
    set_redblack_tree_right(node_ptr, NIL);
}

static uint64_t redblack_tree_search(uint32_t size)
{
    // search logic is the same: rbt & bst
    return bst_internal_find_succ(&rbt, &i_node, (uint64_t)size);
}

/* ------------------------------------- */
/*  Implementation                       */
/* ------------------------------------- */

int redblack_tree_initialize_free_block()
{
    uint64_t first_header = get_firstblock();
    
    // init rbt for block >= 24
    redblack_tree_init();
    set_redblack_tree_parent(first_header, NIL);
    set_redblack_tree_left(first_header, NIL);
    set_redblack_tree_right(first_header, NIL);
    redblack_tree_insert(first_header);

    // init list for small block size == 16
    explist_list_init();

    // init small block list size == 8
    small_list_init();

    return 1;
}

uint64_t redblack_tree_search_free_block(uint32_t payload_size, uint32_t *alloc_blocksize)
{
    // search 8-byte block list
    if (payload_size <= 4)
    {
        // a small block
        *alloc_blocksize = 8;

        if (small_list.count != 0)
        {
            // 8-byte list is not empty
            return small_list.head;
        }
    }
    else
    {
        *alloc_blocksize = round_up(payload_size, 8) + 4 + 4;
    }
    
    // search explicit free list
    if ((*alloc_blocksize) == 16)
    {
        // This search is O(1) search since the list is fixed with size 16
        // if the list is empty, return NIL
        // else, return list head
        uint64_t b16 = explicit_list_search(*alloc_blocksize);
        if (b16 != NIL)
        {
            return b16;
        }
    }

    // search RBT
    return redblack_tree_search(*alloc_blocksize);
}

int redblack_tree_insert_free_block(uint64_t free_header)
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

        case 16:
            explicit_list_insert(free_header);
            break;
        
        default:
            redblack_tree_insert(free_header);
            break;
    }

    return 1;
}

int redblack_tree_delete_free_block(uint64_t free_header)
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

        case 16:
            explicit_list_delete(free_header);
            break;
        
        default:
            redblack_tree_delete(free_header);
            break;
    }

    return 1;
}

void redblack_tree_check_free_block()
{
    small_list_check_free_blocks();
    check_block16_correctness();
}