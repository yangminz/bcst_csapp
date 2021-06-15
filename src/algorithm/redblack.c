#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <headers/algorithm.h>

// shared with BST
rb_node_t *tree_construct(char *str);
void tree_free(rb_node_t *root);

rb_node_t *bst_insert_node(rb_node_t *root, uint64_t val, rb_node_t **inserted);
rb_node_t *bst_delete_node(rb_node_t *root, rb_node_t *n, rb_node_t **replaced);

// 4 kinds of rotations

// return root node
static rb_node_t * rb_rotate_node(rb_node_t *n, rb_node_t *p, rb_node_t *g)
{
    assert(n != NULL && p != NULL && g != NULL);
    assert(n->parent == p && p->parent == g);

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
rb_node_t *rb_delete(rb_node_t *root, rb_node_t *n)
{
    if (n == NULL)
    {
        return NULL;
    }

    // record the color of the to-be-deleted node
    // if it's red, just delete
    // if it's black, may cause a double black situation
    // and we need to do color compensation
    rb_color_t n_color = n->color;

    rb_node_t *v;
    root = bst_delete_node(root, n, &v);

    /****************************************************/
    /* recoloring and restructuring                     */
    /****************************************************/

    // check coloring
    if (n_color == COLOR_RED)
    {
        // when red is removed, safe:
        // 1. root rule - red node is never the root
        // 2. red rule - red's parent & childs are all black
        // 3. black height rule - red is not counted to black height
        return root;
    }
    else
    {
        // black is removed, may violate:
        // 1. root rule - min_upper can be root
        // 2. red rule - min_upper can be a red and its child can be red
        // 3. black height rule - all sub-tree black height deduct 1
        /* 
            g
           /
          p
         / \
        v   s
         */

        rb_node_t *g = NULL;    // grandparent
        rb_node_t *p = NULL;    // sibling
        rb_node_t *s = NULL;    // sibling

        while (1)
        {
            p = v->parent;
            if (v == p->left)
            {
                s = p->right;
            }
            else
            {
                s = p->left;
            }

            if (s != NULL && s->color == COLOR_BLACK)
            {
                // BLACK Silbing

                // At least one of the childs is red - restructuring
                
                if (s->left != NULL && s->left->color == COLOR_RED)
                {
                    // Restructuring Case 1
                    // sibling's left child is red
                    rb_node_t *t = rb_rotate_node(s->left, s, p);
                    t->left->color = COLOR_BLACK;
                    t->right->color = COLOR_BLACK;

                    return root;
                }

                if (s->right != NULL && s->right->color == COLOR_RED)
                {
                    // Restructuring Case 2
                    // sibling's right child is red
                    rb_node_t *t = rb_rotate_node(s->right, s, p);
                    t->left->color = COLOR_BLACK;
                    t->right->color = COLOR_BLACK;

                    return root;
                }

                // Both childs are black - Recoloring
                if (s->left != NULL && s->left->color == COLOR_BLACK && s->right != NULL && s->right->color == COLOR_BLACK)
                {
                    if (p->color == COLOR_RED)
                    {
                        // Recoloring Case 1
                        // parent is red
                        p->color = COLOR_BLACK;
                        s->color = COLOR_RED;

                        return root;
                    }

                    if (p->color == COLOR_BLACK)
                    {
                        // Recoloring Case 2
                        // parent is black
                        s->color = COLOR_RED;

                        // continue
                        v = p;
                        continue;
                    }
                }
            }
            else
            {
                // RED Sibling
                // adjust to black sibling

                int is_p_root = 0;
                if (p->parent == NULL)
                {
                    g = malloc(sizeof(rb_node_t));
                    g->color = COLOR_BLACK;
                    g->left = p;
                    p->parent = g;
                    is_p_root = 1;
                }
                else
                {
                    g = p->parent;
                    is_p_root = 0;
                }

                rb_node_t ** p_addr = NULL;
                if (p == g->left)
                {
                    p_addr = &g->left;
                }
                else
                {
                    p_addr = &g->right;
                }

                if (s == p->right)
                {
                    // case 1: sibling is right
                    p->right = s->right;
                    if (p->right != NULL)
                    {
                        p->right->parent = p;
                    }

                    s->right = p;
                    p->parent = s;
                    p->color = COLOR_RED;

                    *p_addr = s;
                }
                else
                {
                    // case 2: sibling is left
                    p->left = s->left;
                    if (p->left != NULL)
                    {
                        p->left->parent = p;
                    }

                    s->left = p;
                    p->parent = s;
                    p->color = COLOR_RED;

                    *p_addr = s;
                }

                if (is_p_root == 1)
                {
                    free(g);
                    s->parent = NULL;
                }

                // finish the adjustment
                // switch to the BLACK Silbing case
                continue;
            }
        }

    }
    return root;
}

// find the node owning the target value
rb_node_t *rb_find_node(rb_node_t *root, uint64_t val)
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
    printf("Testing Red-Black tree insertion ...\n");

    printf("\tPass\n");
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

    printf("\tPass\n");
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

        if ((0x1 & (i >> 1)) == 0)
        {
            p = g->left;
        }
        else
        {
            p = g->right;
        }

        if ((0x1 & i) == 0)
        {
            n = p->left;
        }
        else
        {
            n = p->right;
        }

        t = rb_rotate_node(n, p, g);
        a = tree_construct(balanced);
        assert(compare_tree(t, a) == 1);

        tree_free(a);
        tree_free(r);
    }
    
    printf("\tPass\n");
}

int main()
{
    test_rotate();
    test_insert();
}

#endif
