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
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "headers/algorithm.h"

/*======================================*/
/*      Base Class Methods              */
/*======================================*/

void linkedlist_base_free(linkedlist_base *list, 
    linkedlist_node_access *node_access)
{
    if (list == NULL)
    {
        return;
    }

    assert(node_access != NULL);

    int count_copy = list->count;
    for (int i = 0; i < count_copy; ++ i)
    {
        uint64_t node = list->head;
        list->update_head(list,
            node_access->get_node_next(list->head));
        
        if (node_access->compare_nodes(node, list->head) == 0)
        {
            // TODO: logic can be removed
            // only one element
            node_access->free_node(node);
            // do not update list->count during deleting
        }
        else
        {
            // at least 2 elements
            uint64_t prev = node_access->get_node_prev(node);
            uint64_t next = node_access->get_node_next(node);

            node_access->set_node_next(prev, next);
            node_access->set_node_prev(next, prev);
            
            node_access->free_node(node);
            // do not update list->count during deleting
        }
    }

    free(list);
}

linkedlist_base *linkedlist_base_add(linkedlist_base *list, 
    linkedlist_node_access *node_access, 
    uint64_t value)
{
    if (list == NULL)
    {
        return NULL;
    }

    assert(node_access != NULL);

    if (list->count == 0)
    {
        // create a new head
        uint64_t head = node_access->alloc_node();
        list->update_head(list, head);
        list->count = 1;
        // circular linked list initialization
        node_access->set_node_prev(head, head);
        node_access->set_node_next(head, head);
        node_access->set_node_value(head, value);
    }
    else
    {
        // insert to head
        uint64_t node = node_access->alloc_node();
        node_access->set_node_value(node, value);
        
        uint64_t head = list->head;
        uint64_t head_prev = node_access->get_node_prev(head);

        node_access->set_node_next(node, head);
        node_access->set_node_prev(head, node);

        node_access->set_node_prev(node, head_prev);
        node_access->set_node_next(head_prev, node);

        list->update_head(list, node);
        list->count ++;
    }

    return list;
}

int linkedlist_base_delete(linkedlist_base *list, 
    linkedlist_node_access *node_access, 
    uint64_t node)
{
    if (list == NULL || node == NULL_ID)
    {
        return 0;
    }

    assert(node_access != NULL);

    // update the prev and next pointers
    // same for the only one node situation
    uint64_t prev = node_access->get_node_prev(node);
    uint64_t next = node_access->get_node_next(node);

    if (prev != NULL_ID)
    {
        node_access->set_node_next(prev, next);
    }

    if (next != NULL_ID)
    {
        node_access->set_node_prev(next, prev);
    }

    // if this node to be free is the head
    if (node_access->compare_nodes(list->head, node) == 0)
    {
        list->update_head(list, next);
    }

    // free the node managed by the list
    node_access->free_node(node);

    // reset the linked list status
    list->count --;

    if (list->count == 0)
    {
        list->update_head(list, NULL_ID);
    }

    return 1;
}

uint64_t linkedlist_base_index(linkedlist_base *list,
    linkedlist_node_access *node_access, 
    uint64_t index)
{
    if (list == NULL || index >= list->count)
    {
        return NULL_ID;
    }

    uint64_t p = list->head;
    for (int i = 0; i <= index; ++ i)
    {
        p = node_access->get_node_next(p);
    }

    return p;
}

// traverse the linked list
uint64_t linkedlist_base_next(linkedlist_base *list,
    linkedlist_node_access *node_access)
{
    if (list == NULL || node_access->compare_nodes(list->head, NULL_ID) == 0)
    {
        return NULL_ID;
    }

    uint64_t old_head = list->head;
    list->update_head(list,
        node_access->get_node_next(old_head));

    return old_head;
}

/*======================================*/
/*      Default Implementation          */
/*======================================*/

// Implementation of the list node access

static uint64_t alloc_node()
{
    return (uint64_t)malloc(sizeof(linkedlist_node_t));
}

static int free_node(uint64_t node_id)
{
    if (node_id == NULL_ID)
    {
        return 0;
    }    
    linkedlist_node_t *node = (linkedlist_node_t *)node_id;
    
    free(node);
    return 1;
}

static int compare_nodes(uint64_t first, uint64_t second)
{
    if (first == second)
    {
        return 0;
    }
    else if (first > second)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}

uint64_t get_node_prev(uint64_t node_id)
{
    if (node_id == NULL_ID)
    {
        return NULL_ID;
    }
    return (uint64_t)(((linkedlist_node_t *)node_id)->prev);
}

int set_node_prev(uint64_t node_id, uint64_t prev_id)
{
    if (node_id == NULL_ID)
    {
        return 0;
    }
    *(uint64_t *)&(((linkedlist_node_t *)node_id)->prev) = prev_id;
    return 1;
}

uint64_t get_node_next(uint64_t node_id)
{
    if (node_id == NULL_ID)
    {
        return NULL_ID;
    }
    return (uint64_t)(((linkedlist_node_t *)node_id)->next);
}

int set_node_next(uint64_t node_id, uint64_t next_id)
{
    if (node_id == NULL_ID)
    {
        return 0;
    }
    *(uint64_t *)&(((linkedlist_node_t *)node_id)->next) = next_id;
    return 1;
}

uint64_t get_node_value(uint64_t node_id)
{
    if (node_id == NULL_ID)
    {
        return NULL_ID;
    }
    return (uint64_t)(((linkedlist_node_t *)node_id)->value);
}

int set_node_value(uint64_t node_id, uint64_t value)
{
    if (node_id == NULL_ID)
    {
        return 0;
    }
    ((linkedlist_node_t *)node_id)->value = value;
    return 1;
}

static linkedlist_node_access node_access =
{
    .alloc_node = &alloc_node,
    .free_node = &free_node,
    .compare_nodes = &compare_nodes,
    .get_node_prev = &get_node_prev,
    .set_node_prev = &set_node_prev,
    .get_node_next = &get_node_next,
    .set_node_next = &set_node_next,
    .get_node_value = &get_node_value,
    .set_node_value = &set_node_value
};

// child class of base class
static int update_head(linkedlist_base *this, uint64_t new_head)
{
    if (this == NULL)
    {
        return 0;
    }
    this->head = new_head;
    return 1;
}

// constructor and destructor
linkedlist_t *linkedlist_construct()
{
    linkedlist_t *list = malloc(sizeof(linkedlist_t));
    list->base.count = 0;
    list->base.head = 0;
    list->base.update_head = &update_head;
    return list;
}

void linkedlist_free(linkedlist_t *list)
{
    linkedlist_base_free(&(list->base), &node_access);
}

linkedlist_t *linkedlist_add(linkedlist_t *list, uint64_t value)
{
    linkedlist_base_add(
        &list->base,
        &node_access,
        value
    );
    return list;
}

int linkedlist_delete(linkedlist_t *list, linkedlist_node_t *node)
{
    return linkedlist_base_delete(
        &list->base,
        &node_access,
        (uint64_t)node
    );
}

linkedlist_node_t *linkedlist_next(linkedlist_t *list)
{
    return (linkedlist_node_t *)linkedlist_base_next(
        &list->base,
        &node_access);
}

linkedlist_node_t *linkedlist_index(linkedlist_t *list, uint64_t index)
{
    return (linkedlist_node_t *)linkedlist_base_index(
        &list->base,
        &node_access,
        index);
}