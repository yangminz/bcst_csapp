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
rb_node_t *tree_construct(char *str);
void tree_print(rb_node_t *root);
void tree_free(rb_node_t *root);

rb_node_t *bst_insert_node(rb_node_t *root, uint64_t val, rb_node_t **inserted);
rb_node_t *bst_delete_node(rb_node_t *root, rb_node_t *n, rb_node_t **replaced);

// 4 kinds of rotations

// return root node
static rb_node_t * rb_rotate_node(rb_node_t *n, rb_node_t *p, rb_node_t *g)
{
    assert(p != NULL && g != NULL);
    assert(p->parent == g);
    if (n != NULL)
    {
        assert(n->parent == p);
    }
    else
    {
        assert(p->left == NULL || p->right == NULL);
    }

    rb_node_t *r = NULL;

    int is_g_root = 0;
    rb_node_t *g_par = NULL;

    if (g->parent == NULL)
    {
        // g is root
        // create a dummy root for g
        g_par = malloc(sizeof(rb_node_t));
        g_par->left = g;
        g->parent = g_par;
        is_g_root = 1;
    }
    else
    {
        // g has parent
        g_par = g->parent;
        is_g_root = 0;
    }

    // the address of g in its parent
    rb_node_t **g_in_par = NULL;
    if (g == g_par->left)
    {
        g_in_par = &(g_par->left);
    }
    else
    {
        g_in_par = &(g_par->right);
    }

    if (n == p->right && p == g->right)
    {
        /*  Left Rotation
            Before Rotation
                 [g]
                 / \
                A  [p]
                   / \
                  B  [n]
                     / \
                    C   D
            After Rotation
               [p]
               / \
             [g] [n]
             /\   /\
            A  B C  D 
            */
        *g_in_par = p;
        p->parent = g_par;

        g->right = p->left;
        if (g->right != NULL)
        {
            g->right->parent = g;
        }

        p->left = g;
        p->left->parent = p;

        r = p;
    }
    else if (n == p->left && p == g->left)
    {
        /*  Right Rotation
            Before Rotation
                 [g]
                 / \
               [p]  D
               / \
             [n]  C
             / \
            A   B
            After Rotation
               [p]
               / \
             [n] [g]
             /\   /\
            A  B C  D 
            */
        *g_in_par = p;
        p->parent = g_par;

        g->left = p->right;
        if (g->left != NULL)
        {
            g->left->parent = g;
        }

        p->right = g;
        p->right->parent = p;

        r = p;
    }
    else if (n == p->right && p == g->left)
    {
        /*  Left-Right Double Rotation
            Before Rotation
                   [g]
                   / \
                 [p]  D
                 / \
                A  [n]
                   / \
                  B   C
            After Rotation
               [n]
               / \
             [p] [g]
             /\   /\
            A  B C  D 
            */
        assert(n != NULL && n->parent == p);
        *g_in_par = n;
        n->parent = g_par;

        p->right = n->left;
        if (p->right != NULL)
        {
            p->right->parent = p;
        }

        g->left = n->right;
        if (g->left != NULL)
        {
            g->left->parent = g;
        }

        n->left = p;
        n->left->parent = n;
        n->right = g;
        n->right->parent = n;

        r = n;
    }
    else if (n == p->left && p == g->right)
    {
        /*  Right-Left Double Rotation
            Before Rotation
                 [g]
                 / \
                A  [p]
                   / \
                  [n] D
                  / \
                 B   C
            After Rotation
               [n]
               / \
             [g] [p]
             /\   /\
            A  B C  D 
            */
        assert(n != NULL && n->parent == p);
        *g_in_par = n;
        n->parent = g_par;

        p->left = n->right;
        if (p->left != NULL)
        {
            p->left->parent = p;
        }

        g->right = n->left;
        if (g->right != NULL)
        {
            g->right->parent = g;
        }

        n->right = p;
        n->right->parent = n;
        n->left = g;
        n->left->parent = n;

        r = n;
    }
    
    if (is_g_root == 1)
    {
        // g_par is a dummy node, we need to free it
        // and we notice that g_par's child may not be g any more
        g_par->left->parent = NULL;
        free(g_par);
    }

    return r;
}

// insert value to the tree
// return the updated tree root node
rb_node_t *rb_insert(rb_node_t *root, uint64_t val)
{
    rb_node_t *n;
    root = bst_insert_node(root, val, &n);

    // fix up the inserted red node (internal node in 2-3-4 tree)
    while (1)
    {
        rb_node_t *p = n->parent;

        if (p->color == COLOR_BLACK)
        {
            // stop fix up
            return root;
        }
        else
        {
            // parent is having red edge, so there must be a grandparent
            rb_node_t *g = p->parent;
            assert(g != NULL);

            if (g->left != NULL && g->left->color == COLOR_RED &&
                g->right != NULL && g->right->color == COLOR_RED)
            {
                // CASE 1: g is have 2 childs and both are red
                // only continue in this case: Promotion
                g->left->color = COLOR_BLACK;
                g->right->color = COLOR_BLACK;
                g->color = COLOR_RED;

                n = g;
                continue;
            }
            else
            {
                // CASE 2: g is having only 1 RED child branch and that's just parent
                rb_node_t *rotate_root = rb_rotate_node(n, p, g);

                // recoloring
                if (rotate_root != NULL)
                {
                    if (rotate_root == p)
                    {
                        p->color = COLOR_BLACK;
                        n->color = COLOR_RED;
                        g->color = COLOR_RED;
                    }
                    else if (rotate_root == n)
                    {
                        n->color = COLOR_BLACK;
                        p->color = COLOR_RED;
                        g->color = COLOR_RED;
                    }

                    if (rotate_root->parent == NULL)
                    {
                        return rotate_root;
                    }
                }

                return root;
            }
        }
    }

    return root;
}

// insert value to the tree
// return the updated tree root node
rb_node_t *rb_delete(rb_node_t *root, uint64_t val)
{
    rb_node_t *x = rb_find(root, val);

    if (x == NULL)
    {
        return NULL;
    }

    // This node db is the parent node with double black child (and this child is NULL)
    rb_node_t *p = NULL;    // parent of db
    rb_node_t *db = NULL;   // db node
    rb_node_t *s = NULL;    // sibling of db
    rb_node_t *n = NULL;    // near child of sibling to db
    rb_node_t *f = NULL;    // far child of sibling to db
    root = bst_delete_node(root, x, &p);

    /****************************************************/
    /* recoloring and restructuring                     */
    /****************************************************/

    // check coloring
    if (p == NULL)
    {
        return root;
    }
    else
    {
        // fix for double black node
        while (1)
        {
            int db_index = p->left == db ? 0 : 1;
            int sb_index = !db_index;
            s = p->childs[sb_index];
            assert(s != NULL);

            if (s->color == COLOR_RED)
            {
                // sibling red, adjust it as black
                rb_node_t *subroot = rb_rotate_node(s->childs[sb_index], s, p);
                if (root == p)
                {
                    // this rotation will update the root
                    root = subroot;
                    root->color = COLOR_BLACK;
                }

                s->color = COLOR_BLACK;
                p->color = COLOR_RED;

                assert(p->parent == s);
                assert(p->childs[db_index] == db);
                assert(s->childs[db_index] == p);

                s = p->childs[sb_index];
            }

            // sibling is black now
            assert(s != NULL && s->color == COLOR_BLACK);
            n = s->childs[db_index];
            f = s->childs[sb_index];

            // try near child first
            if (n != NULL && n->color == COLOR_RED)
            {
                // rotate near child
                rb_node_t *subroot = rb_rotate_node(n, s, p);
                p->color = COLOR_BLACK;

                if (root == p)
                {
                    // this rotation will update the root
                    root = subroot;
                    root->color = COLOR_BLACK;
                }
                return root;
            }

            // try far child then
            if (f != NULL && f->color == COLOR_RED)
            {
                // rotate the far child
                rb_node_t *subroot = rb_rotate_node(f, s, p);
                f->color = p->color;

                if (root == p)
                {
                    // this rotation will update the root
                    root = subroot;
                    root->color = COLOR_BLACK;
                }
                return root;
            }

            // both childs are black
            s->color = COLOR_RED;
            if (p->color == COLOR_RED)
            {
                p->color = COLOR_BLACK;
                return root;
            }
            else
            {
                // parent should be the next double black
                // but we check if it's root first
                if (p->parent == NULL)
                {
                    // p is root
                    return root;
                }
                else
                {
                    db = p;
                    p = db->parent;
                    continue;
                }
            }
        }
    }
    return root;
}

// find the node owning the target value
rb_node_t *rb_find(rb_node_t *root, uint64_t val)
{
    return bst_find(root, val);
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

rb_node_t *rb_tree_construct(char *tree, char *color)
{
    rb_node_t *r = tree_construct(tree);
    int index = color_tree_dfs(r, color, 0);
    assert(index == strlen(color) - 1);

    return r;
}

#ifdef DEBUG_REDBLACK

static int compare_tree(rb_node_t *a, rb_node_t *b)
{
    if (a == NULL && b == NULL)
    {
        return 1;
    }

    if (a == NULL || b == NULL)
    {
        return 0;
    }

    // both not NULL
    if (a->value == b->value && a->color == b->color)
    {
        return  compare_tree(a->left, b->left) && 
                compare_tree(a->right, b->right);
    }
    else
    {
        return 0;
    }
}

static void test_delete()
{
    printf("Testing Red-Black tree deletion ...\n");

    rb_node_t *r;
    rb_node_t *a;

    // no double black
    // bst case 2 - single child
    r = rb_tree_construct(
        "(10,"
            "(5,#,(9,#,#)),"
            "(15,#,#)"
        ")",
        "BB#R##B##");
    r = rb_delete(r, 5);
    a = rb_tree_construct(
        "(10,"
            "(9,#,#),"
            "(15,#,#)"
        ")",
        "BB##B##");
    assert(compare_tree(r, a) == 1);
    tree_free(r);
    tree_free(a);

    // no double black
    // bst case 2 - single child
    r = rb_tree_construct(
        "(10,"
            "(5,(9,#,#),#),"
            "(15,#,#)"
        ")",
        "BBR###B##");
    r = rb_delete(r, 5);
    a = rb_tree_construct(
        "(10,"
            "(9,#,#),"
            "(15,#,#)"
        ")",
        "BB##B##");
    assert(compare_tree(r, a) == 1);
    tree_free(r);
    tree_free(a);

    // no double black
    // bst case 3.1 - 2 childs
    // x->right->left == NULL
    // (R, T1, (B, #, (R, #, #)))
    r = rb_tree_construct(
        "(10,"
            "(5,"   // delete - red
                "(2,#,#),"    // T1
                "(6,#,(7,#,#))"  // successor
            "),"
            "(15,#,#)"  // T1
        ")",
        "BRB##B#R##B##");
    r = rb_delete(r, 5);
    a = rb_tree_construct(
        "(10,"
            "(6,"   // T1
                "(2,#,#),"    // T1
                "(7,#,#)"
            "),"
            "(15,#,#)"  // T1
        ")",
        "BRB##B##B##");
    assert(compare_tree(r, a) == 1);
    tree_free(r);
    tree_free(a);

    // no double black
    // bst case 3.1 - 2 childs
    // x->right->left == NULL
    // (B, T1, (B, #, (R, #, #)))
    r = rb_tree_construct(
        "(10,"
            "(5,"   // delete - black
                "(2,#,#),"    // T1
                "(6,#,(7,#,#))"  // successor
            "),"
            "(15,(12,#,#),(16,#,#))"  // T2
        ")",
        "BBB##B#R##BB##B##");
    r = rb_delete(r, 5);
    a = rb_tree_construct(
        "(10,"
            "(6,"   // T2
                "(2,#,#),"    // T1
                "(7,#,#)"
            "),"
            "(15,(12,#,#),(16,#,#))"  // T2
        ")",
        "BBB##B##BB##B##");
    assert(compare_tree(r, a) == 1);
    tree_free(r);
    tree_free(a);

    // no double black
    // bst case 3.1 - 2 childs
    // (B, T0, (R, #, #))
    r = rb_tree_construct(
        "(10,"
            "(5,"   // delete - black
                "(2,#,#),"    // T0
                "(7,#,#)"  // successor
            "),"
            "(15,#,#)"  // T1
        ")",
        "BBR##R##B##");
    r = rb_delete(r, 5);
    a = rb_tree_construct(
        "(10,"
            "(7,"   // delete - black
                "(2,#,#),"    // T0
                "#"
            "),"
            "(15,#,#)"  // T1
        ")",
        "BBR###B##");
    assert(compare_tree(r, a) == 1);
    tree_free(r);
    tree_free(a);

    // no double black
    // bst case 3.2 - 2 childs
    // successor (R, #, #)
    r = rb_tree_construct(
        "(4,"
            "(2,(1,#,#),(3,#,#)),"
            "(6,"   // delete
                "(5,#,#),"
                "(10,"
                    "(8,(7,#,#),(9,#,#)),"  // 7 successor
                    "(11,#,#)"
                "),"
            ")"
        ")",
        "BBB##B##BB##RBR##R##B##");
    r = rb_delete(r, 6);
    a = rb_tree_construct(
        "(4,"
            "(2,(1,#,#),(3,#,#)),"
            "(7,"   // delete
                "(5,#,#),"
                "(10,"
                    "(8,#,(9,#,#)),"  // 7 successor
                    "(11,#,#)"
                "),"
            ")"
        ")",
        "BBB##B##BB##RB#R##B##");
    assert(compare_tree(r, a) == 1);
    tree_free(r);
    tree_free(a);

    // no double black
    // bst case 3.2 - 2 childs
    // successor (B, #, (R, #, #)), parent red
    r = rb_tree_construct(
        "(8,"
            "(4,(2,(1,#,#),(3,#,#)),(6,(5,#,#),(7,#,#))),"  // T3
            "(12,"   // delete
                "(10,(9,#,#),(11,#,#)),"
                "(17,"
                    "(15,(13,#,(14,#,#)),(16,#,#)),"  // 13 successor
                    "(19,(18,#,#),(20,#,#))"
                "),"
            ")"
        ")",
        "BBBB##B##BB##B##BBB##B##BRB#R##B##RB##B##");
    r = rb_delete(r, 12);
    a = rb_tree_construct(
        "(8,"
            "(4,(2,(1,#,#),(3,#,#)),(6,(5,#,#),(7,#,#))),"  // T3
            "(13,"   // successor
                "(10,(9,#,#),(11,#,#)),"
                "(17,"
                    "(15,(14,#,#),(16,#,#)),"  // recoloring
                    "(19,(18,#,#),(20,#,#))"
                "),"
            ")"
        ")",
        "BBBB##B##BB##B##BBB##B##BRB##B##RB##B##");
    assert(compare_tree(r, a) == 1);
    tree_free(r);
    tree_free(a);

    // no double black
    // bst case 3.2 - 2 childs
    // successor (B, #, (R, #, #)), parent black
    r = rb_tree_construct(
        "(8,"
            "(4,(2,(1,#,#),(3,#,#)),(6,(5,#,#),(7,#,#))),"  // T3
            "(12,"   // delete
                "(10,(9,#,#),(11,#,#)),"
                "(17,"
                    "(15,(13,#,(14,#,#)),(16,#,#)),"  // 13 successor
                    "(19,(18,#,#),(20,#,#))"
                "),"
            ")"
        ")",
        "BBBB##B##BB##B##BBB##B##RBB#R##B##BB##B##");
    r = rb_delete(r, 12);
    a = rb_tree_construct(
        "(8,"
            "(4,(2,(1,#,#),(3,#,#)),(6,(5,#,#),(7,#,#))),"  // T3
            "(13,"   // successor
                "(10,(9,#,#),(11,#,#)),"
                "(17,"
                    "(15,(14,#,#),(16,#,#)),"  // recoloring
                    "(19,(18,#,#),(20,#,#))"
                "),"
            ")"
        ")",
        "BBBB##B##BB##B##BBB##B##RBB##B##BB##B##");
    assert(compare_tree(r, a) == 1);
    tree_free(r);
    tree_free(a);

    // delete red node
    r = rb_tree_construct(
        "(10,"
            "(5,(2,#,#),(9,#,#)),"
            "(30,(25,#,#),(40,(38,#,#),#))"
        ")",
        "BRB##B##RB##BR###");
    r = rb_delete(r, 38);
    a = rb_tree_construct(
        "(10,"
            "(5,(2,#,#),(9,#,#)),"
            "(30,(25,#,#),(40,#,#))"
        ")",
        "BRB##B##RB##B##");
    assert(compare_tree(r, a) == 1);
    tree_free(r);
    tree_free(a);

    // delete black node - simple
    r = rb_tree_construct(
        "(10,"
            "(5,(2,#,#),(9,#,#)),"
            "(30,(25,#,#),(40,(35,#,(38,#,#)),(50,#,#)))"
        ")",
        "BBB##B##BB##RB#R##B##");
    r = rb_delete(r, 30);
    a = rb_tree_construct(
        "(10,"
            "(5,(2,#,#),(9,#,#)),"
            "(35,(25,#,#),(40,(38,#,#),(50,#,#)))"
        ")",
        "BBB##B##BB##RB##B##");
    assert(compare_tree(r, a) == 1);
    tree_free(r);
    tree_free(a);

    // delete a double black node
    // double black = 15
    //  1. sibling black = 30
    //  2. both sibling's childs black = (NULL, NULL)
    // double black gives black to parent
    // parent red, then black
    // sibling red
    r = rb_tree_construct(
        "(10,"
            "(5,#,#),"
            "(20,(15,#,#),(30,#,#))"
        ")",
        "BB##RB##B##");
    r = rb_delete(r, 15);
    a = rb_tree_construct(
        "(10,"
            "(5,#,#),"
            "(20,#,(30,#,#))"
        ")",
        "BB##B#R##");
    assert(compare_tree(r, a) == 1);
    tree_free(r);
    tree_free(a);

    // delete a double black node
    // double black = 15
    //  1. sibling black = 30
    //  2. both sibling's childs black = (NULL, NULL)
    // double black gives black to parent
    // parent black, then parent double black, continue to parent, untill root
    // silbing red
    r = rb_tree_construct(
        "(10,"
            "(5,(1,#,#),(7,#,#)),"
            "(20,(15,#,#),(30,#,#))"
        ")",
        "BBB##B##BB##B##");
    r = rb_delete(r, 15);
    a = rb_tree_construct(
        "(10,"
            "(5,(1,#,#),(7,#,#)),"
            "(20,#,(30,#,#))"
        ")",
        "BRB##B##B#R##");
    assert(compare_tree(r, a) == 1);
    tree_free(r);
    tree_free(a);

    // delete a double black node
    // double black = 15
    //  1. sibling **RED** = 30
    //  2. both sibling's childs black = (NULL, NULL)
    // double black gives black to parent then to sibling
    // parent black, sibling red ==> parent red, sibling black
    r = rb_tree_construct(
        "(10,"
            "(5,(1,#,#),(7,#,#)),"
            "(20,(15,#,#),(30,(25,#,#),(40,#,#)))"
        ")",
        "BBB##B##BB##RB##B##");
    r = rb_delete(r, 15);
    a = rb_tree_construct(
        "(10,"
            "(5,(1,#,#),(7,#,#)),"
            "(30,(20,#,(25,#,#)),(40,#,#))"
        ")",
        "BBB##B##BB#R##B##");
    assert(compare_tree(r, a) == 1);
    tree_free(r);
    tree_free(a);

    // delete
    // sibling black, far child red, near child black
    // sibling black, far child black, near child red
    r = rb_tree_construct(
        "(10,"
            "(5,(1,#,#),(7,#,#)),"
            "(30,(25,(20,#,#),(28,#,#)),(40,#,#))"
        ")",
        "BBB##B##BRB##B##B##");
    r = rb_delete(r, 1);
    a = rb_tree_construct(
        "(25,"
            "(10,(5,#,(7,#,#)),(20,#,#)),"
            "(30,(28,#,#),(40,#,#))"
        ")",
        "BBB#R##B##BB##B##");
    assert(compare_tree(r, a) == 1);
    tree_free(r);
    tree_free(a);

    // A COMPLETE TEST CASE

    // silbing red
    r = rb_tree_construct(
        "(50,"
            "(20,(15,#,#),(35,#,#)),"
            "(65,"
                "(55,#,#),"
                "(70,(68,#,#),(80,#,(90,#,#)))"
            ")"
        ")",
        "BBB##B##BB##RB##B#R##");

    // delete 55 - sibling's 2 black
    r = rb_delete(r, 55);
    a = rb_tree_construct(
        "(50,"
            "(20,(15,#,#),(35,#,#)),"
            "(70,"
                "(65,#,(68,#,#)),"
                "(80,#,(90,#,#))"
            ")"
        ")",
        "BBB##B##BB#R##B#R##");
    assert(compare_tree(r, a) == 1);
    tree_free(a);

    // delete 20 - root double black
    assert(r->left->value == 20);
    r = rb_delete(r, 20);
    a = rb_tree_construct(
        "(50,"
            "(35,(15,#,#),#),"
            "(70,"
                "(65,#,(68,#,#)),"
                "(80,#,(90,#,#))"
            ")"
        ")",
        "BBR###RB#R##B#R##");
    assert(compare_tree(r, a) == 1);
    tree_free(a);
    
    // delete 90 - red node
    assert(r->right->right->right->value == 90);
    r = rb_delete(r, 90);
    a = rb_tree_construct(
        "(50,"
            "(35,(15,#,#),#),"
            "(70,"
                "(65,#,(68,#,#)),"
                "(80,#,#)"
            ")"
        ")",
        "BBR###RB#R##B##");
    assert(compare_tree(r, a) == 1);
    tree_free(a);
    
    // delete 80 - sibling black, near child red, far child black
    assert(r->right->right->value == 80);
    r = rb_delete(r, 80);
    a = rb_tree_construct(
        "(50,"
            "(35,(15,#,#),#),"
            "(68,"
                "(65,#,#),"
                "(70,#,#)"
            ")"
        ")",
        "BBR###RB##B##");
    assert(compare_tree(r, a) == 1);
    tree_free(a);
    
    // delete 50 - root, and having 1B 1R childs, no parent nor sibling
    assert(r->value == 50);
    r = rb_delete(r, 50);
    a = rb_tree_construct(
        "(65,"
            "(35,(15,#,#),#),"
            "(68,#,(70,#,#))"
            ")"
        ")",
        "BBR###B#R##");
    assert(compare_tree(r, a) == 1);
    tree_free(a);
    
    // delete 35 - having red child
    assert(r->left->value == 35);
    r = rb_delete(r, 35);
    a = rb_tree_construct(
        "(65,"
            "(15,#,#),"
            "(68,#,(70,#,#))"
            ")"
        ")",
        "BB##B#R##");
    assert(compare_tree(r, a) == 1);
    tree_free(a);
    
    // delete 15 - far child red sibling black
    assert(r->left->value == 15);
    r = rb_delete(r, 15);
    a = rb_tree_construct(
        "(68,(65,#,#),(70,#,#))",
        "BB##B##");
    assert(compare_tree(r, a) == 1);
    tree_free(a);
    
    // delete 65 - both s-childs black
    assert(r->left->value == 65);
    r = rb_delete(r, 65);
    a = rb_tree_construct(
        "(68,#,(70,#,#))",
        "B#R##");
    assert(compare_tree(r, a) == 1);
    tree_free(a);

    tree_free(r);

    printf("\033[32;1m\tPass\033[0m\n");
}

static void test_insert()
{
    printf("Testing Red-Black tree insertion ...\n");

    rb_node_t *r = rb_tree_construct(
        "(11,"
            "(2,"
                "(1,#,#),"
                "(7,"
                    "(5,#,#),"
                    "(8,#,#)"
                ")"
            "),"
            "(14,#,(15,#,#))"
        ")",
        "B"
            "R"
                "B##"
                "B"
                    "R##"
                    "R##"
            "B#R##");

    // test insert
    r = rb_insert(r, 4);

    // check
    rb_node_t *ans = rb_tree_construct(
        "(7,"
            "(2,"
                "(1,#,#),"
                "(5,(4,#,#),#)"
            "),"
            "(11,"
                "(8,#,#),"
                "(14,#,(15,#,#))"
            ")"
        ")",
        "B"
            "R"
                "B##"
                "BR###"
            "R"
                "B##"
                "B#R##");
    assert(compare_tree(r, ans) == 1);

    tree_free(r);
    tree_free(ans);

    printf("\033[32;1m\tPass\033[0m\n");
}

static void test_rotate()
{
    printf("Testing Red-Black tree rotation ...\n");

    rb_node_t *r;
    rb_node_t *a;
    rb_node_t *t;

    char inputs[8][100] = {
        "(6,"   // g
            "(4,"   // p
                "(2,"   // g
                    "(1,#,#),(3,#,#)),(5,#,#)),(7,#,#))",
        "(6,"   // g
            "(2,"   // p
                "(1,#,#),"
                "(4,"   // n
                    "(3,#,#),(5,#,#))),(7,#,#))",
        "(2,"   // g
            "(1,#,#),"
            "(6,"   // p
                "(4,"   // n
                    "(3,#,#),(5,#,#)),(7,#,#)))",
        "(2,"   // g
            "(1,#,#),"
            "(4,"   // p
                "(3,#,#),"
                "(6,"   // n
                    "(5,#,#),(7,#,#))))",
        "(0,#,"
            "(6,"   // g
                "(4,"   // p
                    "(2,"   // g
                        "(1,#,#),(3,#,#)),(5,#,#)),(7,#,#)))",
        "(0,#,"
            "(6,"   // g
                "(2,"   // p
                    "(1,#,#),"
                    "(4,"   // n
                        "(3,#,#),(5,#,#))),(7,#,#)))",
        "(0,#,"
            "(2,"   // g
                "(1,#,#),"
                "(6,"   // p
                    "(4,"   // n
                        "(3,#,#),(5,#,#)),(7,#,#))))",
        "(0,#,"
            "(2,"   // g
                "(1,#,#),"
                "(4,"   // p
                    "(3,#,#),"
                    "(6,"   // n
                        "(5,#,#),(7,#,#)))))",
    };

    char balanced[100] = "(4,(2,(1,#,#),(3,#,#)),(6,(5,#,#),(7,#,#)))";

    rb_node_t *g = NULL;
    rb_node_t* p = NULL;
    rb_node_t* n = NULL;

    for (int i = 0; i < 8; ++ i)
    {
        r = tree_construct(inputs[i]);

        if ((0x1 & (i >> 2)) == 0)
        {
            // test grandparent root
            g = r;
        }
        else
        {
            // test grandparent not root
            g = r->right;
        }
        p = g->childs[0x1 & (i >> 1)];
        n = p->childs[0x1 & i];

        t = rb_rotate_node(n, p, g);
        a = tree_construct(balanced);
        assert(compare_tree(t, a) == 1);

        tree_free(a);
        tree_free(r);
    }
    
    printf("\033[32;1m\tPass\033[0m\n");
}

int main()
{
    test_rotate();
    test_insert();
    test_delete();
}

#endif
