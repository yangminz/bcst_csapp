#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <headers/algorithm.h>

// 4 kinds of rotations

static void rb_rotate_node(rb_node_t *n)
{
    assert(n != NULL && n->parent != NULL && n->parent->parent != NULL);

    rb_node_t *p = n->parent;
    rb_node_t *g = p->parent;

    int free_g_par = 0;
    rb_node_t *g_par = NULL;

    if (g->parent == NULL)
    {
        // g is root
        // create a dummy root for g
        g_par = malloc(sizeof(rb_node_t));
        g_par->left = g;
        g->parent = g_par;
        free_g_par = 1;
    }
    else
    {
        // g has parent
        g_par = g->parent;
        free_g_par = 0;
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
    }
    
    if (free_g_par == 1)
    {
        // g_par is a dummy node, we need to free it
        // and we notice that g_par's child may not be g any more
        g_par->left->parent = NULL;
        free(g_par);
    }
}

// insert value to the tree
// return the updated tree root node
rb_node_t *rb_insert_node(rb_node_t *root, uint64_t val)
{
    // create
    if (root == NULL)
    {
        root = malloc(sizeof(rb_node_t));
        
        // update properties
        root->parent = NULL;
        root->left = NULL;
        root->right = NULL;
        root->value = val;
        root->color = COLOR_BLACK;  // root node is black

        return root;
    }

    // search the right place to insert data
    rb_node_t *n = root;

    while (n != NULL)
    {
        if (val < n->value)
        {
            if (n->left == NULL)
            {
                // insert here
                n->left = malloc(sizeof(rb_node_t));
                n->left->parent = n;
                n->left->left = NULL;
                n->left->right = NULL;
                n->left->value = val;
                n->left->color = COLOR_RED;

                goto BOTTOM_UP_REBALANCING;
            }
            else
            {
                n = n->left;
            }
        }
        else
        {
            if (n->right == NULL)
            {
                // insert here
                n->right = malloc(sizeof(rb_node_t));
                n->right->parent = n;
                n->right->left = NULL;
                n->right->right = NULL;
                n->right->value = val;
                n->right->color = COLOR_RED;

                goto BOTTOM_UP_REBALANCING;
            }
            else
            {
                n = n->right;
            }
        }
    }

    BOTTOM_UP_REBALANCING:
    // fix up the inserted red node (internal node in 2-3-4 tree)
    while (n != root)
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
                rb_rotate_node(n);

                return root;
            }
        }
    }

    return root;
}

// insert value to the tree
// return the updated tree root node
rb_node_t *rb_delete_node(rb_node_t *root, rb_node_t *target)
{
    if (target == root)
    {
        // no parent pointer
        return NULL;
    }

    // the address of the to be deleted node
    rb_node_t **par_child = NULL;
    if (target == target->parent->left)
    {
        par_child = &(target->parent->left);
    }
    else
    {
        par_child = &(target->parent->right);
    }

    // case 1: single hanging
    if (target->left == NULL && target->right == NULL)
    {
        *par_child = NULL;
        free(target);
        return root;
    }
    
    // case 2: one sub-tree is empty
    // transplant the other sub-tree to the node to be deleted
    if (target->left == NULL && target->right != NULL)
    {
        *par_child = target->right;
        target->right->parent = target->parent;
        free(target);
        return root;
    }

    if (target->left != NULL && target->right == NULL)
    {
        *par_child = target->left;
        target->left->parent = target->parent;
        free(target);
        return root;
    }

    // case 3: both sub-trees are not empty

    // 3.1: a simple remove will do the job
    if (target->right->left == NULL)
    {
        // transplant the left sub-tree
        target->right->left = target->left;
        target->left->parent = target->right;

        *par_child = target->right;
        target->right->parent = target->parent;

        free(target);
        return root;
    }

    // 3.2
    // float up the upper bound as root of sub-tree
    rb_node_t *min_upper = target->right;
    while (min_upper->left != NULL)
    {
        min_upper = min_upper->left;
    }

    // float up min_upper
    min_upper->parent->left = min_upper->right;
    if (min_upper->right != NULL)
    {
        min_upper->right->parent = min_upper->parent;
    }

    // transplant
    min_upper->right = target->right;
    target->right->parent = min_upper;

    min_upper->left = target->left;
    target->left->parent = min_upper;

    min_upper->parent = target->parent;
    
    *par_child = min_upper;
    free(target);
    return root;
}

// find the node owning the target value
rb_node_t *rb_find_node(rb_node_t *root, uint64_t val)
{
    rb_node_t *n = root;
    uint64_t n_value;

    while (n != NULL)
    {
        n_value = n->value;

        if (n_value == val)
        {
            return n;
        }
        else if (val < n_value)
        {
            n = n->left;
        }
        else
        {
            n = n->right;
        }
    }

    return NULL;
}

#ifdef UNIT_TEST

static void dfs_print(rb_node_t *root)
{
    if (root == NULL)
    {
        return;
    }

    char c = root->color == COLOR_RED ? 'R' : 'B';

    dfs_print(root->left);

    printf("color: %c; value: %lu; left: %lu; parent %lu; right: %lu\n", 
        c, root->value, 
        root->left == NULL ? 0 : root->left->value, 
        root->parent == NULL ? 0 : root->parent->value, 
        root->right == NULL ? 0 : root->right->value);

    dfs_print(root->right);
}

static void rb_print(rb_node_t *root)
{
    printf("==================\n");
    dfs_print(root);
}

void test_insert()
{
    // From CLRS chapter 13.3 Red-Black Tree Insertion
    rb_node_t *n1 = malloc(sizeof(rb_node_t));
    rb_node_t *n2 = malloc(sizeof(rb_node_t));
    rb_node_t *n5 = malloc(sizeof(rb_node_t));
    rb_node_t *n7 = malloc(sizeof(rb_node_t));
    rb_node_t *n8 = malloc(sizeof(rb_node_t));
    rb_node_t *n11 = malloc(sizeof(rb_node_t));
    rb_node_t *n14 = malloc(sizeof(rb_node_t));
    rb_node_t *n15 = malloc(sizeof(rb_node_t));

    n1->color = COLOR_BLACK;
    n1->value = 1;
    n1->parent = n2;
    n1->left = NULL;
    n1->right = NULL;

    n2->color = COLOR_RED;
    n2->value = 2;
    n2->parent = n11;
    n2->left = n1;
    n2->right = n7;

    n5->color = COLOR_RED;
    n5->value = 5;
    n5->parent = n7;
    n5->left = NULL;
    n5->right = NULL;

    n7->color = COLOR_BLACK;
    n7->value = 7;
    n7->parent = n2;
    n7->left = n5;
    n7->right = n8;

    n8->color = COLOR_RED;
    n8->value = 8;
    n8->parent = n7;
    n8->left = NULL;
    n8->right = NULL;

    n11->color = COLOR_BLACK;
    n11->value = 11;
    n11->parent = NULL;
    n11->left = n2;
    n11->right = n14;

    n14->color = COLOR_BLACK;
    n14->value = 14;
    n14->parent = n11;
    n14->left = NULL;
    n14->right = n15;

    n15->color = COLOR_RED;
    n15->value = 15;
    n15->parent = n14;
    n15->left = NULL;
    n15->right = NULL;

    // test insert
    rb_node_t *r = rb_insert_node(n11, 8);

    assert(r == n7);

    assert(n1->parent == n2);
    assert(n1->left == NULL);
    assert(n1->right == NULL);
    assert(n1->color == COLOR_BLACK);

    assert(n2->parent == n7);
    assert(n2->left == n1);
    assert(n2->right == n5);
    assert(n2->color == COLOR_RED);

    // insert n4 here
    rb_node_t *n4 = n5->left;
    assert(n4 != NULL);
    assert(n4->parent == n5);
    assert(n4->left == NULL);
    assert(n4->right == NULL);
    assert(n4->color == COLOR_RED);

    assert(n5->parent == n2);
    assert(n5->left != NULL);
    assert(n5->right == NULL);
    assert(n5->color == COLOR_BLACK);

    assert(n7->parent == NULL);
    assert(n7->left == n2);
    assert(n7->right == n11);
    assert(n7->color == COLOR_BLACK);

    assert(n8->parent == n11);
    assert(n8->left == NULL);
    assert(n8->right == NULL);
    assert(n8->color == COLOR_BLACK);

    assert(n11->parent == n7);
    assert(n11->left == n8);
    assert(n11->right == n14);
    assert(n11->color == COLOR_RED);

    assert(n14->parent == n11);
    assert(n14->left == NULL);
    assert(n14->right == n15);
    assert(n14->color == COLOR_BLACK);

    assert(n15->parent == n14);
    assert(n15->left == NULL);
    assert(n15->right == NULL);
    assert(n15->color == COLOR_RED);

    printf("pass insertion test\n");

    free(n1);
    free(n2);
    free(n4);
    free(n5);
    free(n7);
    free(n8);
    free(n11);
    free(n14);
    free(n15);
}

void test_rotate()
{
    // malloc the nodes
    rb_node_t *r = malloc(sizeof(rb_node_t));
    rb_node_t *g = malloc(sizeof(rb_node_t));
    rb_node_t *p = malloc(sizeof(rb_node_t));
    rb_node_t *n = malloc(sizeof(rb_node_t));
    rb_node_t *a = malloc(sizeof(rb_node_t));
    rb_node_t *b = malloc(sizeof(rb_node_t));
    rb_node_t *c = malloc(sizeof(rb_node_t));
    rb_node_t *d = malloc(sizeof(rb_node_t));
    rb_node_t *e = malloc(sizeof(rb_node_t));

    //////////////////////////
    // left rotate 1        //
    //////////////////////////
    g->parent = NULL;
    g->left = a;
    g->right = p;

    p->parent = g;
    p->left = b;
    p->right = n;

    n->parent = p;
    n->left = c;
    n->right = d;

    a->parent = g;
    a->left = NULL;
    a->right = NULL;

    b->parent = p;
    b->left = NULL;
    b->right = NULL;

    c->parent = n;
    c->left = NULL;
    c->right = NULL;
    
    d->parent = n;
    d->left = NULL;
    d->right = NULL;

    rb_rotate_node(n);
    
    assert(p->parent == NULL);
    assert(p->left == g);
    assert(p->right == n);

    assert(g->parent == p);
    assert(g->left == a);
    assert(g->right == b);

    assert(n->parent == p);
    assert(n->left == c);
    assert(n->right == d);

    //////////////////////////
    // left rotate 2        //
    //////////////////////////
    r->parent = NULL;
    r->left = g;
    r->right = NULL;

    g->parent = r;
    g->left = a;
    g->right = p;

    p->parent = g;
    p->left = b;
    p->right = n;

    n->parent = p;
    n->left = c;
    n->right = d;

    a->parent = g;
    a->left = NULL;
    a->right = NULL;

    b->parent = p;
    b->left = NULL;
    b->right = NULL;

    c->parent = n;
    c->left = NULL;
    c->right = NULL;
    
    d->parent = n;
    d->left = NULL;
    d->right = NULL;

    rb_rotate_node(n);
    
    assert(p->parent == r);
    assert(p->left == g);
    assert(p->right == n);

    assert(g->parent == p);
    assert(g->left == a);
    assert(g->right == b);

    assert(n->parent == p);
    assert(n->left == c);
    assert(n->right == d);

    //////////////////////////
    // right rotate 1       //
    //////////////////////////
    g->parent = NULL;
    g->left = p;
    g->right = d;

    p->parent = g;
    p->left = n;
    p->right = c;

    n->parent = p;
    n->left = a;
    n->right = b;

    a->parent = g;
    a->left = NULL;
    a->right = NULL;

    b->parent = p;
    b->left = NULL;
    b->right = NULL;

    c->parent = n;
    c->left = NULL;
    c->right = NULL;
    
    d->parent = n;
    d->left = NULL;
    d->right = NULL;

    rb_rotate_node(n);
    
    assert(p->parent == NULL);
    assert(p->left == n);
    assert(p->right == g);

    assert(n->parent == p);
    assert(n->left == a);
    assert(n->right == b);

    assert(g->parent == p);
    assert(g->left == c);
    assert(g->right == d);

    //////////////////////////
    // right rotate 2       //
    //////////////////////////
    r->parent = NULL;
    r->left = g;
    r->right = NULL;

    g->parent = r;
    g->left = p;
    g->right = d;

    p->parent = g;
    p->left = n;
    p->right = c;

    n->parent = p;
    n->left = a;
    n->right = b;

    a->parent = g;
    a->left = NULL;
    a->right = NULL;

    b->parent = p;
    b->left = NULL;
    b->right = NULL;

    c->parent = n;
    c->left = NULL;
    c->right = NULL;
    
    d->parent = n;
    d->left = NULL;
    d->right = NULL;

    rb_rotate_node(n);
    
    assert(p->parent == r);
    assert(p->left == n);
    assert(p->right == g);

    assert(n->parent == p);
    assert(n->left == a);
    assert(n->right == b);

    assert(g->parent == p);
    assert(g->left == c);
    assert(g->right == d);

    //////////////////////////
    // left right rotate 1  //
    //////////////////////////
    g->parent = NULL;
    g->left = p;
    g->right = d;

    p->parent = g;
    p->left = a;
    p->right = n;

    n->parent = p;
    n->left = b;
    n->right = c;

    a->parent = p;
    a->left = NULL;
    a->right = NULL;

    b->parent = n;
    b->left = NULL;
    b->right = NULL;

    c->parent = n;
    c->left = NULL;
    c->right = NULL;
    
    d->parent = g;
    d->left = NULL;
    d->right = NULL;

    rb_rotate_node(n);

    assert(n->parent == NULL);
    assert(n->left == p);
    assert(n->right == g);
    
    assert(p->parent == n);
    assert(p->left == a);
    assert(p->right == b);

    assert(g->parent == n);
    assert(g->left == c);
    assert(g->right == d);

    //////////////////////////
    // left right rotate 2  //
    //////////////////////////
    r->parent = NULL;
    r->left = g;
    r->right = NULL;

    g->parent = r;
    g->left = p;
    g->right = d;

    p->parent = g;
    p->left = a;
    p->right = n;

    n->parent = p;
    n->left = b;
    n->right = c;

    a->parent = p;
    a->left = NULL;
    a->right = NULL;

    b->parent = n;
    b->left = NULL;
    b->right = NULL;

    c->parent = n;
    c->left = NULL;
    c->right = NULL;
    
    d->parent = g;
    d->left = NULL;
    d->right = NULL;

    rb_rotate_node(n);

    assert(n->parent == r);
    assert(n->left == p);
    assert(n->right == g);
    
    assert(p->parent == n);
    assert(p->left == a);
    assert(p->right == b);

    assert(g->parent == n);
    assert(g->left == c);
    assert(g->right == d);

    //////////////////////////
    // right left rotate 1  //
    //////////////////////////
    g->parent = NULL;
    g->left = a;
    g->right = p;

    p->parent = g;
    p->left = n;
    p->right = d;

    n->parent = p;
    n->left = b;
    n->right = c;

    a->parent = g;
    a->left = NULL;
    a->right = NULL;

    b->parent = n;
    b->left = NULL;
    b->right = NULL;

    c->parent = n;
    c->left = NULL;
    c->right = NULL;
    
    d->parent = p;
    d->left = NULL;
    d->right = NULL;

    rb_rotate_node(n);

    assert(n->parent == NULL);
    assert(n->left == g);
    assert(n->right == p);
    
    assert(g->parent == n);
    assert(g->left == a);
    assert(g->right == b);

    assert(p->parent == n);
    assert(p->left == c);
    assert(p->right == d);

    //////////////////////////
    // right left rotate 2  //
    //////////////////////////
    r->parent = NULL;
    r->left = g;
    r->right = NULL;

    g->parent = r;
    g->left = a;
    g->right = p;

    p->parent = g;
    p->left = n;
    p->right = d;

    n->parent = p;
    n->left = b;
    n->right = c;

    a->parent = g;
    a->left = NULL;
    a->right = NULL;

    b->parent = n;
    b->left = NULL;
    b->right = NULL;

    c->parent = n;
    c->left = NULL;
    c->right = NULL;
    
    d->parent = p;
    d->left = NULL;
    d->right = NULL;

    rb_rotate_node(n);

    assert(n->parent == r);
    assert(n->left == g);
    assert(n->right == p);
    
    assert(g->parent == n);
    assert(g->left == a);
    assert(g->right == b);

    assert(p->parent == n);
    assert(p->left == c);
    assert(p->right == d);

    printf("pass rotation test\n");

    // free all nodes
    free(r);
    free(g);
    free(p);
    free(n);
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
}

int main()
{
    test_rotate();
    test_insert();
}

#endif
