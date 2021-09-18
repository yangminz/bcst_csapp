/* BCST - Introduction to Computer Systems
 * Author:      yangminz@outlook.com
 * Github:      https://github.com/yangminz/bcst_csapp
 * Bilibili:    https://space.bilibili.com/4564101
 * Zhihu:       https://www.zhihu.com/people/zhao-yang-min
 * This project (code repository and videos) is exclusively owned by yangminz 
 * and shall not be used for commercial and profitting purpose 
 * without yangminz's permission.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "headers/algorithm.h"
#include "headers/common.h"

rb_tree_t *bst_construct_keystr(char *str);
int bst_compare(rb_tree_t *a, rb_tree_t *b);

static void test_build()
{
    printf("Testing build tree from string ...\n");

    rb_tree_t *tree;
    rb_node_t *r;
    char s[1000];

    memset(s, 0, sizeof(char) * 1000);
    strcpy(s, "#");
    tree = bst_construct_keystr(s);
    assert(tree->root == NULL_ID);
    bst_free(tree);

    memset(s, 0, sizeof(char) * 1000);
    strcpy(s, "(12, #, #)");
    tree = bst_construct_keystr(s);
    r = (rb_node_t *)tree->root;

    assert(r->parent == NULL);
    assert(r->left == NULL);
    assert(r->right == NULL);
    assert(r->key == 12);
    bst_free(tree);

    memset(s, 0, sizeof(char) * 1000);
    strcpy(s, 
        "("
            "6,"
            "("
                "3,"
                "("
                    "2,"
                    "(1,#,#),"
                    "#"
                "),"
                "("
                    "4,"
                    "#,"
                    "(5,#,#)"
                ")"
            "),"
            "("
                "7,"
                "#,"
                "(8,#,#)"
            ")"
        ")");
    tree = bst_construct_keystr(s);
    r = (rb_node_t *)tree->root;

    rb_node_t *n1 = r->left->left->left;
    rb_node_t *n2 = r->left->left;
    rb_node_t *n3 = r->left;
    rb_node_t *n4 = r->left->right;
    rb_node_t *n5 = r->left->right->right;
    rb_node_t *n6 = r;
    rb_node_t *n7 = r->right;
    rb_node_t *n8 = r->right->right;

    assert(n1->key == 1);
    assert(n1->parent == n2);
    assert(n1->left == NULL);
    assert(n1->right == NULL);

    assert(n2->key == 2);
    assert(n2->parent == n3);
    assert(n2->left == n1);
    assert(n2->right == NULL);

    assert(n3->key == 3);
    assert(n3->parent == n6);
    assert(n3->left == n2);
    assert(n3->right == n4);

    assert(n4->key == 4);
    assert(n4->parent == n3);
    assert(n4->left == NULL);
    assert(n4->right == n5);

    assert(n5->key == 5);
    assert(n5->parent == n4);
    assert(n5->left == NULL);
    assert(n5->right == NULL);

    assert(n6->key == 6);
    assert(n6->parent == NULL);
    assert(n6->left == n3);
    assert(n6->right == n7);

    assert(n7->key == 7);
    assert(n7->parent == n6);
    assert(n7->left == NULL);
    assert(n7->right == n8);

    assert(n8->key == 8);
    assert(n8->parent == n7);
    assert(n8->left == NULL);
    assert(n8->right == NULL);

    bst_free(tree);

    printf("\033[32;1m\tPass\033[0m\n");
}

static void test_insert()
{
    printf("Testing Binary Search tree insertion ...\n");

    rb_tree_t *r = bst_construct_keystr(
        "(11,"
            "(2,(1,#,#),(7,(5,#,#),(8,#,#))),"
            "(14,#,(15,#,#)))"
    );

    // test insert
    bst_add(r, 4);

    // check
    rb_tree_t *a = bst_construct_keystr(
        "(11,"
            "(2,(1,#,#),(7,(5,(4,#,#),#),(8,#,#))),"
            "(14,#,(15,#,#)))"
    );
    assert(bst_compare(r, a) == 1);

    // free
    bst_free(r);
    bst_free(a);
    printf("\033[32;1m\tPass\033[0m\n");
}

int main()
{
    test_build();
    test_insert();
    return 0;
}