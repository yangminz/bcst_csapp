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
/*      Algorithms for Generic          */
/*      Circular Doubly Linked List     */
/*======================================*/

int linkedlist_internal_add(linkedlist_internal_t *list, 
    linkedlist_node_interface *i_node, 
    uint64_t value)
{
    if (list == NULL)
    {
        return 0;
    }
    assert(list->update_head != NULL);
    assert(i_node != NULL);
    assert(i_node->get_node_prev != NULL);
    assert(i_node->set_node_prev != NULL);
    assert(i_node->set_node_next != NULL);
    assert(i_node->construct_node != NULL);
    assert(i_node->set_node_value != NULL);

    uint64_t node = i_node->construct_node();
    i_node->set_node_value(node, value);
    return linkedlist_internal_insert(list, i_node, node);
}

int linkedlist_internal_insert(linkedlist_internal_t *list, 
    linkedlist_node_interface *i_node, 
    uint64_t node)
{
    if (list == NULL)
    {
        return 0;
    }
    assert(list->update_head != NULL);
    assert(i_node != NULL);
    assert(i_node->get_node_prev != NULL);
    assert(i_node->set_node_prev != NULL);
    assert(i_node->set_node_next != NULL);

    if (list->count == 0)
    {
        // create a new head
        list->update_head(list, node);
        list->count = 1;
        // circular linked list initialization
        i_node->set_node_prev(node, node);
        i_node->set_node_next(node, node);
    }
    else
    {
        // insert to head
        uint64_t head = list->head;
        uint64_t head_prev = i_node->get_node_prev(head);

        i_node->set_node_next(node, head);
        i_node->set_node_prev(head, node);

        i_node->set_node_prev(node, head_prev);
        i_node->set_node_next(head_prev, node);

        list->update_head(list, node);
        list->count ++;
    }

    return 1;
}

int linkedlist_internal_delete(linkedlist_internal_t *list, 
    linkedlist_node_interface *i_node, 
    uint64_t node)
{
    if (list == NULL || node == NULL_ID)
    {
        return 0;
    }
    assert(list->update_head != NULL);
    assert(i_node != NULL);
    assert(i_node->destruct_node != NULL);
    assert(i_node->compare_nodes != NULL);
    assert(i_node->is_null_node != NULL);
    assert(i_node->get_node_prev != NULL);
    assert(i_node->set_node_prev != NULL);
    assert(i_node->get_node_next != NULL);
    assert(i_node->set_node_next != NULL);

    // update the prev and next pointers
    // same for the only one node situation
    uint64_t prev = i_node->get_node_prev(node);
    uint64_t next = i_node->get_node_next(node);

    if (i_node->is_null_node(prev) == 0)
    {
        i_node->set_node_next(prev, next);
    }

    if (i_node->is_null_node(next) == 0)
    {
        i_node->set_node_prev(next, prev);
    }

    // if this node to be free is the head
    if (i_node->compare_nodes(list->head, node) == 0)
    {
        list->update_head(list, next);
    }

    // free the node managed by the list
    i_node->destruct_node(node);

    // reset the linked list status
    list->count --;

    if (list->count == 0)
    {
        list->update_head(list, NULL_ID);
    }

    return 1;
}

uint64_t linkedlist_internal_index(linkedlist_internal_t *list,
    linkedlist_node_interface *i_node, 
    uint64_t index)
{
    if (list == NULL || index >= list->count)
    {
        return NULL_ID;
    }
    assert(i_node != NULL);
    assert(i_node->get_node_next != NULL);

    uint64_t p = list->head;
    for (int i = 0; i <= index; ++ i)
    {
        p = i_node->get_node_next(p);
    }

    return p;
}

// traverse the linked list
uint64_t linkedlist_internal_next(linkedlist_internal_t *list,
    linkedlist_node_interface *i_node)
{
    if (list == NULL || i_node->compare_nodes(list->head, NULL_ID) == 0)
    {
        return NULL_ID;
    }
    assert(list->update_head != NULL);
    assert(i_node != NULL);
    assert(i_node->get_node_next != NULL);

    uint64_t old_head = list->head;
    list->update_head(list,
        i_node->get_node_next(old_head));

    return old_head;
}

/*======================================*/
/*      Default Implementation          */
/*======================================*/

// Implementation of the list node access

static int is_null_node(uint64_t node_id)
{
    if (node_id == NULL_ID)
    {
        return 1;
    }
    return 0;
}

static uint64_t construct_node()
{
    return (uint64_t)malloc(sizeof(linkedlist_node_t));
}

static int destruct_node(uint64_t node_id)
{
    if (is_null_node(node_id) == 1)
    {
        return 0;
    }    
    linkedlist_node_t *node = (linkedlist_node_t *)node_id;
    
    free(node);
    return 1;
}

static int compare_nodes(uint64_t first, uint64_t second)
{
    return !(first == second);
}

static uint64_t get_node_prev(uint64_t node_id)
{
    if (is_null_node(node_id) == 1)
    {
        return NULL_ID;
    }
    return (uint64_t)(((linkedlist_node_t *)node_id)->prev);
}

static int set_node_prev(uint64_t node_id, uint64_t prev_id)
{
    if (is_null_node(node_id) == 1)
    {
        return 0;
    }
    *(uint64_t *)&(((linkedlist_node_t *)node_id)->prev) = prev_id;
    return 1;
}

static uint64_t get_node_next(uint64_t node_id)
{
    if (is_null_node(node_id) == 1)
    {
        return NULL_ID;
    }
    return (uint64_t)(((linkedlist_node_t *)node_id)->next);
}

static int set_node_next(uint64_t node_id, uint64_t next_id)
{
    if (is_null_node(node_id) == 1)
    {
        return 0;
    }
    *(uint64_t *)&(((linkedlist_node_t *)node_id)->next) = next_id;
    return 1;
}

static uint64_t get_node_value(uint64_t node_id)
{
    if (is_null_node(node_id) == 1)
    {
        return NULL_ID;
    }
    return (uint64_t)(((linkedlist_node_t *)node_id)->value);
}

static int set_node_value(uint64_t node_id, uint64_t value)
{
    if (is_null_node(node_id) == 1)
    {
        return 0;
    }
    ((linkedlist_node_t *)node_id)->value = value;
    return 1;
}

static linkedlist_node_interface i_node =
{
    .construct_node = &construct_node,
    .destruct_node = &destruct_node,
    .is_null_node = &is_null_node,
    .compare_nodes = &compare_nodes,
    .get_node_prev = &get_node_prev,
    .set_node_prev = &set_node_prev,
    .get_node_next = &get_node_next,
    .set_node_next = &set_node_next,
    .get_node_value = &get_node_value,
    .set_node_value = &set_node_value
};

// child class of base class
static int update_head(linkedlist_internal_t *this, uint64_t new_head)
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
    list->base.head = NULL_ID;
    list->base.update_head = &update_head;
    return list;
}

void linkedlist_free(linkedlist_t *list)
{
    if (list == NULL)
    {
        return;
    }
    linkedlist_internal_t *base = &(list->base);
    assert(base->update_head != NULL);

    int count_copy = base->count;
    for (int i = 0; i < count_copy; ++ i)
    {
        uint64_t node = base->head;
        base->update_head(base,
            i_node.get_node_next(base->head));
        
        if (i_node.compare_nodes(node, base->head) == 0)
        {
            // TODO: logic can be removed
            // only one element
            i_node.destruct_node(node);
            // do not update list->count during deleting
        }
        else
        {
            // at least 2 elements
            uint64_t prev = i_node.get_node_prev(node);
            uint64_t next = i_node.get_node_next(node);

            i_node.set_node_next(prev, next);
            i_node.set_node_prev(next, prev);
            
            i_node.destruct_node(node);
            // do not update list->count during deleting
        }
    }

    free(list);
}

void linkedlist_add(linkedlist_t *list, uint64_t value)
{
    linkedlist_internal_add(
        &list->base,
        &i_node,
        value
    );
}

void linkedlist_delete(linkedlist_t *list, linkedlist_node_t *node)
{
    linkedlist_internal_delete(
        &list->base,
        &i_node,
        (uint64_t)node
    );
}

linkedlist_node_t *linkedlist_next(linkedlist_t *list)
{
    return (linkedlist_node_t *)linkedlist_internal_next(
        &list->base,
        &i_node);
}

linkedlist_node_t *linkedlist_index(linkedlist_t *list, uint64_t index)
{
    return (linkedlist_node_t *)linkedlist_internal_index(
        &list->base,
        &i_node,
        index);
}