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

static int internal_heap_init();
void tree_internal_print(rbtree_internal_t *tree, rbtree_node_interface *i_node);

/* ------------------------------------- */
/*  Implementation of the Interfaces     */
/* ------------------------------------- */

#ifdef REDBLACK_TREE

int heap_init()
{
    return internal_heap_init();
}
#endif

#define MIN_REDBLACK_TREE_BLOCKSIZE (24)

/* ------------------------------------- */
/*  Operations for Tree Block Structure  */
/* ------------------------------------- */

static int destruct_node(uint64_t header_vaddr) 
{
    // do nothing here
    return 1;
}

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
    assert((header_vaddr & 0x3) == 0x0);
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
    assert((header_vaddr & 0x3) == 0x0);
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

// The explicit free linked list
static rbtree_internal_t rbt;

static void redblack_tree_init()
{
    rbt.root = NULL_ID;
    rbt.update_root = &update_root;
}

static void redblack_tree_insert(uint64_t node_ptr)
{
    // BST for now
    set_redblack_tree_parent(node_ptr, NIL);
    set_redblack_tree_left(node_ptr, NIL);
    set_redblack_tree_right(node_ptr, NIL);
    set_redblack_tree_color(node_ptr, COLOR_RED);

    bst_internal_insert(&rbt, &i_node, node_ptr);
}

static void redblack_tree_delete(uint64_t node_ptr)
{
    // BST for now
    bst_internal_delete(&rbt, &i_node, node_ptr, 0);

    set_redblack_tree_parent(node_ptr, NIL);
    set_redblack_tree_left(node_ptr, NIL);
    set_redblack_tree_right(node_ptr, NIL);
}

static uint64_t redblack_tree_search(uint32_t size)
{
    // BST for now
    return bst_internal_find_succ(&rbt, &i_node, (uint64_t)size);
}

/* ------------------------------------- */
/*  For Debugging                        */
/* ------------------------------------- */


/* ------------------------------------- */
/*  Implementation                       */
/* ------------------------------------- */

static int internal_heap_init()
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
    set_allocated(prologue_header, ALLOCATED);

    uint64_t prologue_footer = prologue_header + 4;
    set_blocksize(prologue_footer, 8);
    set_allocated(prologue_footer, ALLOCATED);

    // set the epilogue block
    // it's a footer only
    uint64_t epilogue = get_epilogue();
    set_blocksize(epilogue, 0);
    set_allocated(epilogue, ALLOCATED);

    // set the block size & allocated of the only regular block
    uint64_t first_header = get_firstblock();
    set_blocksize(first_header, 4096 - 4 - 8 - 4);
    set_allocated(first_header, FREE);
    set_redblack_tree_color(first_header, COLOR_BLACK);

    uint64_t first_footer = get_footer(first_header);
    set_blocksize(first_footer, 4096 - 4 - 8 - 4);
    set_allocated(first_footer, FREE);
    set_redblack_tree_color(first_footer, COLOR_BLACK);

    set_redblack_tree_parent(first_header, NULL_ID);
    set_redblack_tree_left(first_header, NULL_ID);
    set_redblack_tree_right(first_header, NULL_ID);

    redblack_tree_init();
    redblack_tree_insert(first_header);

    return 1;
}