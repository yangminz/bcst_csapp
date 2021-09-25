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
#include <headers/algorithm.h>

// shared with BST
rbtree_node_interface default_i_rbt_node;
rb_tree_t *bst_construct_keystr(char *str);
int internal_tree_compare(uint64_t a, uint64_t b, rbtree_node_interface *i_node, int is_rbt);

// 4 kinds of rotations
static void rbt_internal_rotate(uint64_t n, uint64_t p, uint64_t g,
    rbtree_node_interface *i_node)
{
}

void rbt_internal_insert(rbtree_internal_t *tree,
    rbtree_node_interface *i_node, 
    uint64_t node_id)
{
}

void rbt_internal_delete(rbtree_internal_t *tree,
    rbtree_node_interface *i_node, 
    uint64_t node_id)
{
}

/*======================================*/
/*      Default Implementation          */
/*======================================*/

rb_tree_t *rbt_construct()
{
    return bst_construct();
}

// build binary tree
static int color_tree_dfs(rb_node_t *n, char *color, int index)
{
    if (n == NULL)
    {
        assert(color[index] == '#');
        return index;
    }

    if (color[index] == 'R')
    {
        n->color = COLOR_RED;
    }
    else if (color[index] == 'B')
    {
        n->color = COLOR_BLACK;
    }

    index = color_tree_dfs(n->left, color, index + 1);
    index = color_tree_dfs(n->right, color, index + 1);

    return index;
}

rb_tree_t *rbt_construct_keystr(char *tree, char *color)
{
    rb_tree_t *t = bst_construct_keystr(tree);

    if (t == NULL)
    {
        return NULL;
    }

    uint64_t root_id = t->root;
    if (default_i_rbt_node.is_null_node(root_id) == 1)
    {
        return t;
    }

    // root is not NULL
    // color the red-black tree
    rb_node_t *root_ptr = (rb_node_t *)root_id;
    int index = color_tree_dfs(root_ptr, color, 0);
    assert(index == strlen(color) - 1);

    return t;
}

void rbt_free(rb_tree_t *tree)
{
    bst_free(tree);
}

void rbt_add(rb_tree_t *tree, uint64_t key)
{
    rb_node_t *n = (rb_node_t *)default_i_rbt_node.construct_node();
    n->key = key;

    rbt_insert(tree, n);
}

void rbt_insert(rb_tree_t *tree, rb_node_t *node)
{
    rbt_internal_insert(&(tree->base), &default_i_rbt_node, (uint64_t)node);
}

void rbt_remove(rb_tree_t *tree, uint64_t key)
{
    rb_node_t *node = rbt_find(tree, key);
    if (node == NULL)
    {
        return;
    }
    rbt_delete(tree, node);
}

void rbt_delete(rb_tree_t *tree, rb_node_t *node)
{
    rbt_internal_delete(&(tree->base), &default_i_rbt_node, (uint64_t)node);
}

rb_node_t *rbt_find(rb_tree_t *tree, uint64_t key)
{
    return bst_find(tree, key);
}

rb_node_t *rbt_find_succ(rb_tree_t *tree, uint64_t key)
{
    return rbt_find_succ(tree, key);
}

int rbt_compare(rb_tree_t *a, rb_tree_t *b)
{
    if (a == NULL && b == NULL)
    {
        return 1;
    }
    if (a == NULL || b == NULL)
    {
        return 0;
    }
    
    return internal_tree_compare(a->root, b->root, &default_i_rbt_node, 1);
}

void rbt_rotate(rb_node_t *n, rb_node_t *p, rb_node_t *g)
{
    rbt_internal_rotate((uint64_t)n, (uint64_t)p, (uint64_t)g, &default_i_rbt_node);
}