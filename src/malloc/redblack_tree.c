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

static int internal_heap_init();
static uint64_t internal_malloc(uint32_t size);
static void internal_free(uint64_t payload_vaddr);

/* ------------------------------------- */
/*  Implementation of the Interfaces     */
/* ------------------------------------- */

#ifdef REDBLACK_TREE

int heap_init()
{
    return internal_heap_init();
}

uint64_t mem_alloc(uint32_t size)
{
    return internal_malloc(size);
}

void mem_free(uint64_t payload_vaddr)
{
    internal_free(payload_vaddr);
}

#ifdef DEBUG_MALLOC
void on_sigabrt(int signum)
{
    // like a try-catch for the asserts
    printf("%s\n", debug_message);
    print_heap();
    exit(0);
}
#endif

#endif

/* ------------------------------------- */
/*  Red-Black Tree Free Blocks           */
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

#define MIN_BINARY_TREE_BLOCKSIZE (20)

#define BLACK (0)
#define RED (1)

static uint64_t redblack_tree_root_node = NIL;

/* ------------------------------------- */
/*  Operations for Tree Block Structure  */
/* ------------------------------------- */

static uint32_t get_redblack_tree_prev(uint64_t header_vaddr)
{
    return get_field32_block_ptr(header_vaddr, MIN_BINARY_TREE_BLOCKSIZE, 4);
}

static uint32_t get_redblack_tree_left(uint64_t header_vaddr)
{
    return get_field32_block_ptr(header_vaddr, MIN_BINARY_TREE_BLOCKSIZE, 8);
}

static uint32_t get_redblack_tree_right(uint64_t header_vaddr)
{
    return get_field32_block_ptr(header_vaddr, MIN_BINARY_TREE_BLOCKSIZE, 12);
}

static uint32_t get_redblack_tree_color(uint64_t header_vaddr)
{
    if (header_vaddr == NIL)
    {
        // default BLACK
        return BLACK;
    }

    assert(get_prologue() <= header_vaddr && header_vaddr <= get_epilogue());
    assert((header_vaddr & 0x3) == 0x0);  // header & footer should be 4 bytes alignment

    uint32_t header_value = *(uint32_t *)&heap[header_vaddr];
    return ((header_value >> 1) & 0x1);
}

static void set_redblack_tree_prev(uint64_t header_vaddr, uint64_t prev_vaddr)
{
    set_field32_block_ptr(header_vaddr, prev_vaddr, MIN_BINARY_TREE_BLOCKSIZE, 4);
}

static void set_redblack_tree_left(uint64_t header_vaddr, uint64_t left_vaddr)
{
    set_field32_block_ptr(header_vaddr, left_vaddr, MIN_BINARY_TREE_BLOCKSIZE, 8);
}

static void set_redblack_tree_right(uint64_t header_vaddr, uint64_t right_vaddr)
{
    set_field32_block_ptr(header_vaddr, right_vaddr, MIN_BINARY_TREE_BLOCKSIZE, 12);
}

static void set_redblack_tree_color(uint64_t header_vaddr, uint32_t color)
{
    if (header_vaddr == NIL)
    {
        return;
    }

    assert(color == BLACK || color == RED);
    assert(get_prologue() <= header_vaddr && header_vaddr <= get_epilogue());
    assert((header_vaddr & 0x3) == 0x0);  // header & footer should be 4 bytes alignment

    *(uint32_t *)&heap[header_vaddr] &= 0xFFFFFFFD;
    *(uint32_t *)&heap[header_vaddr] |= ((color & 0x1) << 1);
}

static rbtree_node_interface i_node = 
{
    .construct_node = NULL,
    .destruct_node = NULL,
    .compare_nodes = NULL,
    .get_parent = &get_redblack_tree_prev,
    .set_parent = &set_redblack_tree_prev,
    .get_leftchild = &get_redblack_tree_left,
    .set_leftchild = &set_redblack_tree_left,
    .get_rightchild = &get_redblack_tree_right,
    .set_rightchild = &set_redblack_tree_right,
    .get_color = &get_redblack_tree_color,
    .set_color = &set_redblack_tree_color,
};

/* ------------------------------------- */
/*  Operations for Red-Black Tree        */
/* ------------------------------------- */

static void redblack_tree_insert(uint64_t tree_root, uint64_t node_ptr)
{
    // TODO: implement insert
}

static void redblack_tree_delete(uint64_t tree_root, uint64_t node_ptr)
{
    // TODO: implement delete
}

static void redblack_tree_search(uint64_t tree_root, uint32_t size)
{
    // TODO: implement search
}

/* ------------------------------------- */
/*  For Debugging                        */
/* ------------------------------------- */


/* ------------------------------------- */
/*  Implementation                       */
/* ------------------------------------- */

static int internal_heap_init()
{
    // TODO
    return 0;
}

static uint64_t internal_malloc(uint32_t size)
{
    // TODO
    return 0;
}

static void internal_free(uint64_t payload_addr)
{
    // TODO
}