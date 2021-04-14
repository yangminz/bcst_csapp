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

typedef void (*cleanup_t)();

static cleanup_t **events = NULL;
static int ecount = 0;
static int esize = 0;

void add_cleanup_event(void *func)
{
    assert(func != 0);
    cleanup_t *e = (cleanup_t *)func;

    // lazy malloc
    if (esize == 0 && events == NULL)
    {
        // uninitialized
        esize = 8;   // start from 8 events
        events = malloc(sizeof(cleanup_t *) * esize);

        // fill in the first event
        events[0] = e;
        ecount += 1;
        return;
    }

    if (ecount == esize)
    {
        // extend - double size
        esize *= 2;
        cleanup_t **temp = malloc(sizeof(cleanup_t *) * esize);

        // copy the old to the new
        for (int i = 0; i < ecount; ++ i)
        {
            temp[i] = events[i];
        }
        // add the new one
        temp[ecount] = e;
        ecount ++;

        // free the old array
        free(events);
        events = temp;
    }
    else
    {
        events[ecount] = e;
        ecount ++;
    }
}

void finally_cleanup()
{
    for (int i = 0; i < ecount; ++ i)
    {
        (*events[i])();
    }

    // clean itself
    free(events);
}