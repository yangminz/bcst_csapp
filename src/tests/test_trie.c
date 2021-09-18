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
#include "headers/algorithm.h"

void trie_print(trie_node_t *root);

static void test_insert()
{
    printf("Testing Trie ...\n");

    trie_node_t *root = trie_construct();
    uint64_t result;

    root = trie_insert(root, "abcd", 12);
    root = trie_insert(root, "ab", 108);
    root = trie_insert(root, "abef", 1022);

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

    printf("\033[32;1m\tPass\033[0m\n");
}

int main()
{
    test_insert();
}