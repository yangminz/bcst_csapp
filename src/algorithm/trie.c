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
#include <string.h>
#include "headers/cpu.h"
#include "headers/memory.h"
#include "headers/common.h"
#include "headers/algorithm.h"

// constructor
trie_node_t * trie_construct()
{
    trie_node_t *root = malloc(sizeof(trie_node_t));
    root->next = hashtable_construct(8);
    root->isvalue = 0;
    root->value = 0;
    return root;
}

// free
void trie_free(trie_node_t *root)
{
    // two ways:
    // 1. like the mark-sweep algorithm
    // 2. recursive *
    if (root == NULL)
    {
        return;
    }

    // free sub trees
    hashtable_t *next_table = root->next;

    if (next_table != NULL)
    {
        // sub trees
        for (int i = 0; i < next_table->num; ++ i)
        {
            hashtable_bucket_t *b = next_table->directory[i];
            for (int j = 0; j < b->counter; ++ j)
            {
                uint64_t subtree_addr = b->varray[j];
                trie_free((trie_node_t *)subtree_addr);
            }
        }

        // free this hash table
        hashtable_free(next_table);
    }

    // free root node
    free(root);
}

int trie_insert(trie_node_t **address, char *key, uint64_t value)
{
    trie_node_t *p = *address;
    if (p == NULL)
    {
        return 0;
    }

    for (int i = 0; i < strlen(key); ++ i)
    {
        if (p->next == NULL)
        {
            p->next = hashtable_construct(8);
        }

        // char as hash key
        char hashkey[2];
        hashkey[0] = key[i];
        hashkey[1] = '\0';

        uint64_t trie_node_addr;

        if (hashtable_get(p->next, hashkey, &trie_node_addr) == 1)
        {
            // found next node in this root
            p = (trie_node_t *)trie_node_addr;
            continue;
        }
        else
        {
            // no next node in this root
            // create new node
            trie_node_t *n = malloc(sizeof(trie_node_t));
            n->value = 0;
            n->isvalue = 0;
            n->next = NULL;     // leaf node has no next

            hashtable_insert(&p->next, hashkey, (uint64_t)n);

            // goto next node
            p = n;
            continue;
        }
    }

    // may overwrite
    p->value = value;
    p->isvalue = 1;
    return 1;
}

int trie_get(trie_node_t *root, char *key, uint64_t *valptr)
{
    trie_node_t *p = root;
    for (int i = 0; i < strlen(key); ++ i)
    {
        if (p == NULL)
        {
            // not found
            return 0;
        }

        // go to next
        if (p->next != NULL)
        {
            char hashkey[2];
            hashkey[0] = key[i];
            hashkey[1] = '\0';

            uint64_t trie_node_addr;
            if (hashtable_get(p->next, hashkey, &trie_node_addr) == 1)
            {
                // found next node
                p = (trie_node_t *)trie_node_addr;
                continue;
            }
            else
            {
                // not found next node
                return 0;
            }
        }
        else
        {
            // should have mapping here 
            return 0;
        }
    }

    if (p->isvalue == 1)
    {
        *valptr = p->value;
        return 1;
    }

    return 0;
}

#if (defined UNIT_TEST) || (defined DEBUG_DATASTRUCTURE)

static void trie_dfs_print(trie_node_t *x, int level, char c)
{
    if (x != NULL)
    {
        if (level > 0)
        {
            for (int i = 0; i < level - 1; ++ i)
            {
                printf("\t");
            }
            printf("[%c] %p\n", c, x);
        }

        for (int j = 0; j < x->next->num; ++ j)
        {
            hashtable_bucket_t *b = x->next->directory[j];
            for (int k = 0; k < b->counter; ++ k)
            {
                trie_node_t *subtree = (trie_node_t *)b->varray[k];
                char *key = b->karray[k];
                assert(strlen(key) >= 1);
                trie_dfs_print(subtree, level + 1, key[0]);
            }
        }
    }
}

void trie_print(trie_node_t *root)
{
    if ((DEBUG_VERBOSE_SET & DEBUG_DATASTRUCTURE) == 0)
    {
        return;
    }

    if (root == NULL)
    {
        printf("NULL\n");
    }
    else
    {
        printf("Print Trie:\n");
    }

    trie_dfs_print(root, 0, 0);
}

static void test_insert()
{
    trie_node_t *root = trie_construct();
    uint64_t result;

    trie_insert(&root, "abcd", 12);
    trie_insert(&root, "ab", 108);
    trie_insert(&root, "abef", 1022);

    assert(trie_get(root, "abcd", &result) == 1);
    assert(result == 12);
    assert(trie_get(root, "ab", &result) == 1);
    assert(result == 108);
    assert(trie_get(root, "abef", &result) == 1);
    assert(result == 1022);

    assert(trie_get(root, "a", &result) == 0);
    assert(trie_get(root, "abc", &result) == 0);
    assert(trie_get(root, "b", &result) == 0);
    assert(trie_get(root, "x", &result) == 0);

    trie_free(root);

    printf("trie pass insert\n");
}

int main()
{
    test_insert();
}

#endif
