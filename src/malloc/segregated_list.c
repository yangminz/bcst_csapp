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