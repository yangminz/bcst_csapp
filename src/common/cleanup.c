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
#include "headers/common.h"
#include "headers/algorithm.h"

typedef void (*cleanup_t)();

static array_t *events = NULL;

void add_cleanup_event(void *func)
{
    assert(func != NULL);

    if (events == NULL)
    {
        // uninitialized - lazy malloc
        // start from 8 slots
        events = array_construct(8);
    }

    // fill in the first event
    array_insert(&events, (uint64_t)func);
    return;
}

void finally_cleanup()
{
    for (int i = 0; i < events->count; ++ i)
    {
        uint64_t address;
        assert(array_get(events, i, &address) != 0);

        cleanup_t *func;
        *(uint64_t *)&func = address;
        (*func)();
    }

    // clean itself
    array_free(events);
    events = NULL;
}