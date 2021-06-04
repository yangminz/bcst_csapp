#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <headers/algorithm.h>

// 4 kinds of rotations

void rb_rotate_node(rb_node_t *n)
{
    assert(n != NULL && n->parent != NULL && n->parent->parent != NULL);

    rb_node_t *p = n->parent;
    rb_node_t *g = p->parent;

    if (g->parent == NULL)
    {
        // TODO: g is root
    }
    else
    {
        // g has parent
        rb_node_t **g_ref = NULL;
        if (g == g->parent->left)
        {
            g_ref = &(g->parent->left);
        }
        else
        {
            g_ref = &(g->parent->right);
        }

        if (n == p->right && p == g->right)
        {
            // left rotate
            *g_ref = p;
            p->parent = g->parent;

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
            // right rotate
            *g_ref = p;
            p->parent = g->parent;

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
            // left-right double rotate
            *g_ref = n;
            n->parent = g->parent;

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
            // right-left double rotate
            *g_ref = n;
            n->parent = g->parent;

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

                goto FIXUP;
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

                goto FIXUP;
            }
            else
            {
                n = n->right;
            }
        }
    }

    FIXUP:
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
                // only continue in this case
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
    rb_node_t *p = root;
    uint64_t p_value;

    while (p != NULL)
    {
        p_value = p->value;

        if (p_value == val)
        {
            return p;
        }
        else if (val < p_value)
        {
            p = p->left;
        }
        else
        {
            p = p->right;
        }
    }

    return NULL;
}

void dfs_print(rb_node_t *root)
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

void rb_print(rb_node_t *root)
{
    printf("==================\n");
    dfs_print(root);
}

void test_insert()
{
    rb_node_t *root = NULL;

    root = rb_insert_node(root, 6);
    root = rb_insert_node(root, 3);
    root = rb_insert_node(root, 8);
    root = rb_insert_node(root, 2);
    root = rb_insert_node(root, 4);
    root = rb_insert_node(root, 1);
    root = rb_insert_node(root, 5);
    root = rb_insert_node(root, 7);
    root = rb_insert_node(root, 11);
    root = rb_insert_node(root, 9);
    root = rb_insert_node(root, 10);
    root = rb_insert_node(root, 12);
    root = rb_insert_node(root, 13);

    rb_print(root);

    rb_node_t *n = rb_find_node(root, 8);    
    root = rb_delete_node(root, n);
    
    rb_print(root);
}

void test_right_rotate()
{
    rb_node_t *r = malloc(sizeof(rb_node_t));
    r->value = 10;

    // g
    r->left = malloc(sizeof(rb_node_t));
    rb_node_t *g = r->left;

    g->value = 9;
    g->parent = r;

    // p
    g->left = malloc(sizeof(rb_node_t));
    rb_node_t *p = g->left;
    
    p->value = 8;
    p->parent = g;

    // n
    p->left = malloc(sizeof(rb_node_t));
    rb_node_t *n = p->left;

    n->value = 7;
    n->parent = p;

    rb_print(r);

    rb_rotate_node(n);

    rb_print(r);
}

void test_left_rotate()
{
    rb_node_t *r = malloc(sizeof(rb_node_t));
    r->value = 10;

    // g
    r->right = malloc(sizeof(rb_node_t));
    rb_node_t *g = r->right;

    g->value = 9;
    g->parent = r;

    // p
    g->right = malloc(sizeof(rb_node_t));
    rb_node_t *p = g->right;
    
    p->value = 8;
    p->parent = g;

    // n
    p->right = malloc(sizeof(rb_node_t));
    rb_node_t *n = p->right;

    n->value = 7;
    n->parent = p;

    rb_print(r);

    rb_rotate_node(n);

    rb_print(r);
}

void test_leftright_rotate()
{
    rb_node_t *r = malloc(sizeof(rb_node_t));
    r->value = 10;

    // g
    r->left = malloc(sizeof(rb_node_t));
    rb_node_t *g = r->left;

    g->value = 9;
    g->parent = r;

    // p
    g->left = malloc(sizeof(rb_node_t));
    rb_node_t *p = g->left;
    
    p->value = 8;
    p->parent = g;

    // n
    p->right = malloc(sizeof(rb_node_t));
    rb_node_t *n = p->right;

    n->value = 7;
    n->parent = p;

    rb_print(r);

    rb_rotate_node(n);

    rb_print(r);
}

void test_rightleft_rotate()
{
    rb_node_t *r = malloc(sizeof(rb_node_t));
    r->value = 10;

    // g
    r->right = malloc(sizeof(rb_node_t));
    rb_node_t *g = r->right;

    g->value = 9;
    g->parent = r;

    // p
    g->right = malloc(sizeof(rb_node_t));
    rb_node_t *p = g->right;
    
    p->value = 8;
    p->parent = g;

    // n
    p->left = malloc(sizeof(rb_node_t));
    rb_node_t *n = p->left;

    n->value = 7;
    n->parent = p;

    rb_print(r);

    rb_rotate_node(n);

    rb_print(r);
}

int main()
{
    test_left_rotate();
    test_right_rotate();
    test_leftright_rotate();
    test_rightleft_rotate();
}