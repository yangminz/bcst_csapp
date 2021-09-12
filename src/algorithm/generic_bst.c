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

void bstree_internal_insert(rbtree_internal_t *tree,
    rbtree_node_interface *i_node, 
    uint64_t node_id)
{
    if (tree == NULL)
    {
        return;
    }
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

    if (i_node->is_null_node(tree->root) == 1)
    {
        i_node->set_parent(node_id, NULL_ID);
        i_node->set_leftchild(node_id, NULL_ID);
        i_node->set_rightchild(node_id, NULL_ID);
        i_node->set_color(node_id, COLOR_BLACK);

        tree->update_root(tree, node_id);
        return;
    }
    
    uint64_t p = tree->root;
    uint64_t n_key = i_node->get_key(node_id);

    while (i_node->is_null_node(p) == 0)
    {
        uint64_t p_key = i_node->get_key(p);

        if (n_key < p_key)
        {
            uint64_t p_left = i_node->get_leftchild(p);

            if (i_node->is_null_node(p_left) == 1)
            {
                // insert node to p->left
                i_node->set_leftchild(p, node_id);
                return;
            }
            else
            {
                p = p_left;
            }
        }
        else // if (n_key > p_key)
        {
            uint64_t p_right = i_node->get_rightchild(p);

            if (i_node->is_null_node(p_right) == 1)
            {
                // insert node to p->left
                i_node->set_rightchild(p, node_id);
                return;
            }
            else
            {
                p = p_right;
            }
        }
    }
}

void bstree_internal_delete(rbtree_internal_t *tree,
    rbtree_node_interface *i_node, 
    uint64_t node_id)
{
    // TODO
}

uint64_t bstree_internal_find(rbtree_internal_t *tree, 
    rbtree_node_interface *i_node, 
    uint64_t value)
{
    // TODO
}

#ifdef DEBUG_BST

int main()
{
    return 0;
}

#endif
