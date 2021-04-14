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
#include "headers/datastruct.h"

static int get_index(char c)
{
    if (c == '%')
    {
        return 36;
    }
    else if ('0' <= c && c <= '9')
    {
        return c - '0';
    }
    else if ('a' <= c && c <= 'z')
    {
        return c - 'a' + 10;
    }
    return -1;
}

static char get_char(int id)
{
    assert(0 <= id && id <= 36);
    if (id == 36)
    {
        return '%';
    }
    else if (0 <= id && id <= 9)
    {
        return (char)('0' + id);
    }
    else if (10 <= id && id <= 35)
    {
        return (char)('a' + id - 10);
    }
    return '?';
}

void trie_insert(trie_node_t **root, char *key, uint64_t val)
{
    trie_node_t **p = root;
    for (int i = 0; i < strlen(key); ++ i)
    {
        if (*p == NULL)
        {
            *p = malloc(sizeof(trie_node_t));
        }

        p = &((*p)->next[get_index(key[i])]);
    }
    if (*p == NULL)
    {
        *p = malloc(sizeof(trie_node_t));
    }

    // may overwrite
    (*p)->address = val;
}

int trie_get(trie_node_t *root, char *key, uint64_t *val)
{
    trie_node_t *p = root;
    for (int i = 0; i < strlen(key); ++ i)
    {
        if (p == NULL)
        {
            // not found
            return 0;
        }
        p = p->next[get_index(key[i])];
    }
    *val = p->address;
    return 1;
}

void trie_free(trie_node_t *root)
{
    // two ways:
    // 1. like the mark-sweep algorithm
    // 2. recursive *
    if (root == NULL)
    {
        return;
    }
    for (int i = 0; i <= 36; ++ i)
    {
        trie_free(root->next[i]);
    }
    free(root);
}

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

        for (int j = 0; j <= 36; ++ j)
        {
            trie_dfs_print(x->next[j], level + 1, get_char(j));
        }
    }
}

void trie_print(trie_node_t *root)
{
    if ((DEBUG_VERBOSE_SET & DEBUG_PARSEINST) == 0)
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