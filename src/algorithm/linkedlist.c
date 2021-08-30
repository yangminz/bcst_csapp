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
#include <stdio.h>
#include <stdint.h>
#include "headers/algorithm.h"

// constructor and destructor
linkedlist_t *linkedlist_construct()
{
    linkedlist_t *list = malloc(sizeof(linkedlist_t));
    list->count = 0;
    list->head = NULL;
    return list;
}

void linkedlist_free(linkedlist_t *list)
{
    if (list == NULL)
    {
        return;
    }

    // listptr is the STACK ADDRESS storing the referenced address
    for (int i = 0; i < list->count; ++ i)
    {
        linkedlist_node_t *node = list->head;
        list->head = list->head->next;
        if (node == list->head)
        {
            // only one element
            free(node);
            // do not update list->count during deleting
        }
        else
        {
            // at least 2 elements
            node->prev->next = node->next;
            node->next->prev = node->prev;
            free(node);
            // do not update list->count during deleting
        }
    }
    free(list);
}

linkedlist_t *linkedlist_add(linkedlist_t *list, uint64_t value)
{
    if (list == NULL)
    {
        return NULL;
    }

    if (list->count == 0)
    {
        // create a new head
        list->head = malloc(sizeof(linkedlist_node_t));
        // circular linked list initialization
        list->head->prev = list->head;
        list->head->next = list->head;
        list->head->value = value;
        list->count = 1;
    }
    else
    {
        // insert to tail to implement FIFO policy
        linkedlist_node_t *node = malloc(sizeof(linkedlist_node_t));
        node->value = value;
        node->next = list->head;
        node->prev = list->head->prev;
        node->next->prev = node;
        node->prev->next = node;
        list->count ++;
    }
    return list;
}

int linkedlist_delete(linkedlist_t *list, linkedlist_node_t *node)
{
    if (list == NULL || list->count == 0)
    {
        return 0;
    }

    // update the prev and next pointers
    // same for the only one node situation
    node->prev->next = node->next;
    node->next->prev = node->prev;

    // if this node to be free is the head
    if (node == list->head)
    {
        list->head = node->next;
    }

    // free the node managed by the list
    free(node);
    // reset the linked list status
    list->count --;

    if (list->count == 0)
    {
        list->head = NULL;
    }
    return 1;
}

linkedlist_node_t *linkedlist_get(linkedlist_t *list, uint64_t value)
{
    if (list == NULL)
    {
        return NULL;
    }

    linkedlist_node_t *p = list->head;
    for (int i = 0; i < list->count; ++ i)
    {
        if (p->value == value)
        {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

linkedlist_node_t *linkedlist_index(linkedlist_t *list, uint64_t index)
{
    if (list == NULL || index >= list->count)
    {
        return NULL;
    }

    linkedlist_node_t *p = list->head;
    for (int i = 0; i <= index; ++ i)
    {
        p = p->next;
    }
    return p;
}

// traverse the linked list
linkedlist_node_t *linkedlist_next(linkedlist_t *list)
{
    if (list == NULL)
    {
        return NULL;
    }
    list->head = list->head->next;
    return list->head->prev;
}