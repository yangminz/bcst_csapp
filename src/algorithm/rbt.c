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

static void swap_node_content(uint64_t a, uint64_t b, 
    rbtree_node_interface *i_node)
{
    assert(i_node != NULL);
    assert(i_node->is_null_node != NULL);
    assert(i_node->set_color != NULL);
    assert(i_node->set_key != NULL);
    assert(i_node->set_value != NULL);
    assert(i_node->get_color != NULL);
    assert(i_node->get_key != NULL);
    assert(i_node->get_value != NULL);

    assert(i_node->is_null_node(a) == 0);
    assert(i_node->is_null_node(b) == 0);

    uint64_t a_key = i_node->get_key(a);
    uint64_t a_value = i_node->get_value(a);
    rb_color_t a_color = i_node->get_color(a);

    uint64_t b_key = i_node->get_key(b);
    uint64_t b_value = i_node->get_value(b);
    rb_color_t b_color = i_node->get_color(b);

    i_node->set_key(a, b_key);
    i_node->set_value(a, b_value);
    i_node->set_color(a, b_color);

    i_node->set_key(b, a_key);
    i_node->set_value(b, a_value);
    i_node->set_color(b, a_color);
}

// 4 kinds of rotations
static void rbt_internal_rotate(uint64_t n, uint64_t p, uint64_t g,
    rbtree_node_interface *i_node)
{
    assert(i_node != NULL);
    assert(i_node->is_null_node != NULL);
    assert(i_node->set_parent != NULL);
    assert(i_node->set_leftchild != NULL);
    assert(i_node->set_rightchild != NULL);
    assert(i_node->set_color != NULL);
    assert(i_node->set_key != NULL);
    assert(i_node->get_parent != NULL);
    assert(i_node->get_leftchild != NULL);
    assert(i_node->get_rightchild != NULL);
    assert(i_node->get_color != NULL);
    assert(i_node->get_key != NULL);

    // MUST NOT be NULL
    assert(i_node->is_null_node(n) == 0);
    assert(i_node->is_null_node(p) == 0);
    assert(i_node->is_null_node(g) == 0);

    // MUST be parent and grandparent
    assert(i_node->compare_nodes(p, i_node->get_parent(n)) == 0);
    assert(i_node->compare_nodes(g, i_node->get_parent(p)) == 0);

    uint64_t n_left = i_node->get_leftchild(n);
    uint64_t n_right = i_node->get_rightchild(n);
    uint64_t p_left = i_node->get_leftchild(p);
    uint64_t p_right = i_node->get_rightchild(p);
    uint64_t g_left = i_node->get_leftchild(g);
    uint64_t g_right = i_node->get_rightchild(g);

    if (i_node->compare_nodes(g_left, p) == 0)
    {
        if (i_node->compare_nodes(p_left, n) == 0)
        {
            // update pointers
            i_node->set_leftchild(g, n);
            i_node->set_parent(n, g);

            i_node->set_rightchild(g, p);

            i_node->set_leftchild(p, p_right);
            i_node->set_rightchild(p, g_right);
            if (i_node->is_null_node(g_right) == 0)
            {
                i_node->set_parent(g_right, p);
            }
            
            swap_node_content(g, p, i_node);
        }
        else
        {
            // update pointers
            i_node->set_rightchild(g, n);
            i_node->set_parent(n, g);

            i_node->set_rightchild(p, n_left);
            if (i_node->is_null_node(n_left) == 0)
            {
                i_node->set_parent(n_left, p);
            }

            i_node->set_leftchild(n, n_right);
            i_node->set_rightchild(n, g_right);
            if (i_node->is_null_node(g_right) == 0)
            {
                i_node->set_parent(g_right, n);
            }

            swap_node_content(g, n, i_node);
        }
    }
    else
    {
        if (i_node->compare_nodes(p_left, n) == 0)
        {
            // update pointers
            i_node->set_leftchild(g, n);
            i_node->set_parent(n, g);

            i_node->set_leftchild(p, n_right);
            if (i_node->is_null_node(n_right) == 0)
            {
                i_node->set_parent(n_right, p);
            }

            i_node->set_rightchild(n, n_left);
            i_node->set_leftchild(n, g_left);
            if (i_node->is_null_node(g_left) == 0)
            {
                i_node->set_parent(g_left, n);
            }

            swap_node_content(g, n, i_node);
        }
        else
        {
            // update pointers
            i_node->set_rightchild(g, n);
            i_node->set_parent(n, g);

            i_node->set_leftchild(g, p);

            i_node->set_rightchild(p, p_left);
            i_node->set_leftchild(p, g_left);
            if (i_node->is_null_node(g_left) == 0)
            {
                i_node->set_parent(g_left, p);
            }
            
            swap_node_content(g, p, i_node);
        }
    }
}

void rbt_internal_insert(rbtree_internal_t *tree,
    rbtree_node_interface *i_node, 
    uint64_t node_id)
{
    if (tree == NULL)
    {
        return;
    }
    assert(tree->update_root != NULL);
    assert(i_node->is_null_node != NULL);
    assert(i_node->set_parent != NULL);
    assert(i_node->set_leftchild != NULL);
    assert(i_node->set_rightchild != NULL);
    assert(i_node->set_color != NULL);
    assert(i_node->set_key != NULL);
    assert(i_node->get_parent != NULL);
    assert(i_node->get_leftchild != NULL);
    assert(i_node->get_rightchild != NULL);
    assert(i_node->get_color != NULL);
    assert(i_node->get_key != NULL);

    assert(i_node->is_null_node(node_id) == 0);

    // set the inserted node as red node
    i_node->set_color(node_id, COLOR_RED);
    i_node->set_parent(node_id, NULL_ID);
    i_node->set_leftchild(node_id, NULL_ID);
    i_node->set_rightchild(node_id, NULL_ID);

    // if tree is empty, node_id would be inserted as BLACK node
    bst_internal_insert(tree, i_node, node_id);

    uint64_t n = node_id;
    // float up RBT
    while (1)
    {
        rb_color_t n_color = i_node->get_color(n);

        uint64_t p = i_node->get_parent(n);
        if (i_node->is_null_node(p) == 0)
        {
            rb_color_t p_color = i_node->get_color(p);

            assert(n_color == COLOR_RED);
            if (p_color == COLOR_BLACK)
            {
                // end of floating up
                return;
            }
            else
            {
                // p is red && n is red
                // ==> g exists and its is black
                uint64_t g = i_node->get_parent(p);
                assert(i_node->is_null_node(g) == 0);
                assert(i_node->get_color(g) == COLOR_BLACK);

                // rotate
                rbt_internal_rotate(n, p, g, i_node);

                // recolor
                // the root of {g, p, n} subtree is g!
                i_node->set_color(g, COLOR_RED);
                i_node->set_color(p, COLOR_BLACK);
                i_node->set_color(n, COLOR_BLACK);

                n = g;
                continue;
            }
        }
        else
        {
            // n should be the root of the tree
            i_node->set_color(n, COLOR_BLACK);
            return;
        }
    }
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