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

#define MIN_IMPLICIT_FREE_LIST_BLOCKSIZE (8)

#ifdef IMPLICIT_FREE_LIST

int initialize_free_block()
{
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
    *alloc_blocksize = free_blocksize;

    // search the whole heap
    uint64_t b = get_firstblock();
    while (b <= get_lastblock())
    {
        uint32_t b_blocksize = get_blocksize(b);
        uint32_t b_allocated = get_allocated(b);

        if (b_allocated == FREE && free_blocksize <= b_blocksize)
        {
            return b;
        }
        else
        {
            b = get_nextheader(b);
        }
    }

    return NIL;
}

int insert_free_block(uint64_t free_header)
{
    return 1;
}

int delete_free_block(uint64_t free_header)
{
    return 1;
}

void check_freeblock_correctness()
{
}
#endif