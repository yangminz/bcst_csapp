/* BCST - Introduction to Computer Systems
 * Author:      yangminz@outlook.com
 * Github:      https://github.com/yangminz/bcst_csapp
 * Bilibili:    https://space.bilibili.com/4564101
 * Zhihu:       https://www.zhihu.com/people/zhao-yang-min
 * This project (code repository and videos) is exclusively owned by yangminz 
 * and shall not be used for commercial and profitting purpose 
 * without yangminz's permission.
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "headers/common.h"
#include "headers/datastruct.h"

static uint64_t compute_tag(char *str);
static void tag_destroy();

typedef struct
{
    // tag is like the generation in GC
    // or it's like the mark process in mark-sweep algorithm
    // but we manually mark the memory block with tag instead
    // since C cannot distinguish the pointer and data correctly
    uint64_t tag;
    uint64_t size;
    void *ptr;
} tag_block_t;

// one draw back of this implementation is that the tag is dependent on linkedlist
// so linkedlist cannot use tag_*() functions
// and also cleanup events (array) cannot use tag_*()
static linkedlist_t *tag_list = NULL;

void *tag_malloc(uint64_t size, char *tagstr)
{
    uint64_t tag = compute_tag(tagstr);

    tag_block_t *b = malloc(sizeof(tag_block_t));
    b->tag = tag;
    b->size = size;
    b->ptr = malloc(size);

    // manage this block
    if (tag_list == NULL)
    {
        tag_list = linkedlist_construct();
        // remember to clean the tag_list finally
        add_cleanup_event(&tag_destroy);
    }
    // add the heap address to the managing list
    linkedlist_add(&tag_list, (uint64_t)b);

    return b->ptr;
}

int tag_free(void *ptr)
{
    int found = 0;
    // it's very slow because we are managing it
    for (int i = 0; i < tag_list->count; ++ i)
    {
        linkedlist_node_t *p = linkedlist_next(tag_list);

        tag_block_t *b = (tag_block_t *)p->value;
        if (b->ptr == ptr)
        {
            // found this block
            linkedlist_delete(tag_list, p);
            // free block
            free(b);
            found = 1;
            break;
        }
    }

    if (found == 0)
    {
        // Or we should exit the process at once?
        return 0;
    }

    free(ptr);
    return 1;
}

void tag_sweep(char *tagstr)
{
    // sweep all the memory with target tag
    // NOTE THAT THIS IS VERY DANGEROUS SINCE IT WILL FREE ALL TAG MEMORY
    // CALL THIS FUNCTION ONLY IF YOU ARE VERY CONFIDENT
    uint64_t tag = compute_tag(tagstr);

    for (int i = 0; i < tag_list->count; ++ i)
    {
        linkedlist_node_t *p = linkedlist_next(tag_list);

        tag_block_t *b = (tag_block_t *)p->value;
        if (b->tag == tag)
        {
            // free heap memory
            free(b->ptr);
            // free block
            free(b);
            // free from the linked list
            linkedlist_delete(tag_list, p);
        }
    }
}

static void tag_destroy()
{    
    for (int i = 0; i < tag_list->count; ++ i)
    {
        linkedlist_node_t *p = linkedlist_next(tag_list);
        tag_block_t *b = (tag_block_t *)p->value;

        // free heap memory
        free(b->ptr);
        // free block
        free(b);
        // free from the linked list
        linkedlist_delete(tag_list, p);
    }
    linkedlist_free(tag_list);
}

// just copy it from hashtable.c
static uint64_t compute_tag(char *str)
{
    int p = 31;
    int m = 1000000007;

    int k = p;
    int v = 0;
    for (int i = 0; i < strlen(str); ++ i)
    {
        v = (v + ((int)str[i] * k) % m) % m;
        k = (k * p) % m;
    }
    return v;
}