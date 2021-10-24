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

/*
    yangminz:   I will not implement the segregated free list solution.
                This file will not be built either. It's only used to 
                show the basic ideas of segregated free list.
 */

static linkedlist_internal_t segregated_lists[11];

static int get_list_index(uint64_t blocksize)
{
    if (blocksize < 8)
    {
        return 0;
    }

    if (blocksize >= 4096)
    {
        return 10;
    }
    
    uint64_t low_bound = 8;
    for (int i = 1; i < 10; ++ i)
    {
        if (low_bound <= blocksize && blocksize < 2 * low_bound)
        {
            return i;
        }
        low_bound *= 2;
    }

    return -1;
}

void check_size_list_correctness(
    linkedlist_internal_t *list, linkedlist_node_interface *i_node, 
    uint32_t min_size, uint32_t max_size)
{
    uint32_t counter = 0;
    uint64_t b = get_firstblock(); 
    int head_exists = 0;
    while(b <= get_lastblock())
    {
        uint32_t bsize = get_blocksize(b);
        if (get_allocated(b) == FREE && min_size <= bsize && bsize <= max_size)
        {
            uint64_t prev = i_node->get_node_prev(b);
            uint64_t next = i_node->get_node_next(b);
            uint64_t prev_next = i_node->get_node_next(prev);
            uint64_t next_prev = i_node->get_node_prev(next);

            assert(get_allocated(prev) == FREE);
            assert(get_allocated(next) == FREE);
            assert(prev_next == b);
            assert(next_prev == b);

            if (b == list->head)
            {
                head_exists = 1;
            }

            counter += 1;
        }
        b = get_nextheader(b);
    }
    assert(list->count == 0 || head_exists == 1);
    assert(counter == list->count);

    uint64_t p = list->head;
    uint64_t n = list->head;
    for (int i = 0; i < list->count; ++ i)
    {
        uint32_t psize = get_blocksize(p);
        uint32_t nsize = get_blocksize(n);

        assert(get_allocated(p) == FREE);
        assert(min_size <= psize && psize <= max_size);

        assert(get_allocated(n) == FREE);
        assert(min_size <= nsize && nsize <= max_size);

        p = i_node->get_node_prev(p);
        n = i_node->get_node_next(n);
    }
    assert(p == list->head);
    assert(n == list->head);
}