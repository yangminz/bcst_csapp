#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <headers/algorithm.h>
#include <headers/common.h>

// sentinel to indicate NULL
// its color IS BLACK!!! (for red-black tree to use)
static rb_node_t NULL_TREE_NODE = {
    .color = COLOR_BLACK,
    .value = 0,
    .left = NULL,
    .right = NULL,
    .parent = NULL
};
rb_node_t *NULL_TREE_NODE_PTR = &NULL_TREE_NODE;

rb_node_t *bst_insert_node(rb_node_t *root, uint64_t val, rb_node_t **redblack);
rb_node_t *bst_delete_node(rb_node_t *root, rb_node_t *n, rb_node_t **redblack);

// insert value to the tree
// return the updated tree root node
rb_node_t *bst_insert(rb_node_t *root, uint64_t val)
{
    rb_node_t *redblack;
    return bst_insert_node(root, val, &redblack);
}

rb_node_t *bst_insert_node(rb_node_t *root, uint64_t val, rb_node_t **redblack)
{
    // create
    if (root == NULL_TREE_NODE_PTR)
    {
        root = malloc(sizeof(rb_node_t));
        
        // update properties
        root->parent = NULL_TREE_NODE_PTR;
        root->left = NULL_TREE_NODE_PTR;
        root->right = NULL_TREE_NODE_PTR;
        root->value = val;
        root->color = COLOR_BLACK;

        *redblack = root;
        return root;
    }

    // search the right place (leaf node) to insert data
    rb_node_t *n = root;

    while (n != NULL_TREE_NODE_PTR)
    {
        if (val < n->value)
        {
            if (n->left == NULL_TREE_NODE_PTR)
            {
                // insert here
                n->left = malloc(sizeof(rb_node_t));
                n->left->parent = n;
                n->left->left = NULL_TREE_NODE_PTR;
                n->left->right = NULL_TREE_NODE_PTR;
                n->left->value = val;
                n->left->color = COLOR_RED;
                
                *redblack = n->left;
                return root;
            }
            else
            {
                n = n->left;
            }
        }
        else if (val > n->value)
        {
            if (n->right == NULL_TREE_NODE_PTR)
            {
                // insert here
                n->right = malloc(sizeof(rb_node_t));
                n->right->parent = n;
                n->right->left = NULL_TREE_NODE_PTR;
                n->right->right = NULL_TREE_NODE_PTR;
                n->right->value = val;
                n->right->color = COLOR_RED;

                *redblack = n->right;
                return root;
            }
            else
            {
                n = n->right;
            }
        }
        else
        {
            // equals
            printf("bst insertion: existing value {%lx} being tried to redblack.\n", val);
            exit(0);
        }
    }

    *redblack = NULL_TREE_NODE_PTR;
    return root;
}

rb_node_t *bst_delete(rb_node_t *root, rb_node_t *n)
{
    rb_node_t **redblack;
    return bst_delete_node(root, n, &redblack);
}

// delete the node
// return the updated tree root node
rb_node_t *bst_delete_node(rb_node_t *root, rb_node_t *n, rb_node_t **redblack)
{
    if (n == NULL_TREE_NODE_PTR)
    {
        return NULL_TREE_NODE_PTR;
    }

    // in case root is deleted
    rb_node_t *p = NULL_TREE_NODE_PTR;
    int is_n_root = 0;
    if (n == root)
    {
        // no parent pointer, create a dummy one
        p = malloc(sizeof(rb_node_t));
        p->color = COLOR_BLACK;
        p->left = n;
        n->parent = p;
        p->parent = NULL_TREE_NODE_PTR;
        p->right = NULL_TREE_NODE_PTR;
        is_n_root = 1;
    }
    else
    {
        p = n->parent;
        is_n_root = 0;
    }

    if (n->left == NULL_TREE_NODE_PTR && n->right == NULL_TREE_NODE_PTR)
    {
        //////////////////////////////////////////////
        // case 1: leaf node                        //
        //////////////////////////////////////////////
        free(n);
        
        // for red-black tree, the current node is NULL
        *redblack = NULL_TREE_NODE_PTR;

        if (is_n_root == 1)
        {
            // tree is empty now
            root = NULL_TREE_NODE_PTR;
        }
    }
    else if (n->left != NULL_TREE_NODE_PTR || n->right != NULL_TREE_NODE_PTR)
    {
        //////////////////////////////////////////////
        // case 2: one sub-tree is empty            //
        //////////////////////////////////////////////

        // transplant the not-empty sub-tree to the node to be deleted
        // 0 - left; 1 - right
        int child_index = n->left == NULL_TREE_NODE_PTR ? 1 : 0;

        n->childs[child_index]->parent = p;
        free(n);

        if (is_n_root == 1)
        {
            root = n->childs[child_index];
        }
    }
    else if (n->right->left == NULL_TREE_NODE_PTR)
    {
        //////////////////////////////////////////////
        // case 3: both sub-trees are not empty     //
        // 3.1: a simple remove will do the job     //
        //////////////////////////////////////////////

        // transplant the left sub-tree
        n->right->left = n->left;
        n->left->parent = n->right;

        *par_child = n->right;
        n->right->parent = p;

        free(n);

        if (is_n_root == 1)
        {
            root = n->right;
        }
    }
    else if (n->right->left != NULL_TREE_NODE_PTR)
    {
        //////////////////////////////////////////////
        // 3.2: float up the tight upper bound      //
        // as root of sub-tree                      //
        //////////////////////////////////////////////

        rb_node_t *min_upper = n->right;
        while (min_upper->left != NULL_TREE_NODE_PTR)
        {
            min_upper = min_upper->left;
        }

        // float up min_upper
        min_upper->parent->left = min_upper->right;
        if (min_upper->right != NULL_TREE_NODE_PTR)
        {
            min_upper->right->parent = min_upper->parent;
        }

        // transplant
        min_upper->right = n->right;
        n->right->parent = min_upper;

        min_upper->left = n->left;
        n->left->parent = min_upper;

        min_upper->parent = p;
        
        *par_child = min_upper;
        free(n);

        if (is_n_root == 1)
        {
            root = min_upper;
        }
    }
    
    if (is_n_root == 1)
    {
        free(p);

        if (root != NULL_TREE_NODE_PTR)
        {
            root->parent = NULL_TREE_NODE_PTR;
        }
    }

    // which node takes the place of the deleted node in parent
    *redblack = *par_child;
    return root;
}

// find the node owning the target value
rb_node_t *bst_find(rb_node_t *root, uint64_t val)
{
    rb_node_t *n = root;
    uint64_t n_value;

    while (n != NULL_TREE_NODE_PTR)
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

    return NULL_TREE_NODE_PTR;
}

void tree_free(rb_node_t *root)
{
    if (root == NULL_TREE_NODE_PTR)
    {
        return;
    }

    tree_free(root->left);
    tree_free(root->right);

    free(root);
}

rb_node_t *tree_construct(char *str)
{
    // (root node, left tree, right tree)
    // for NULL_TREE_NODE_PTR node, #

    // sentinel to mark the unprocessed sub-tree
    rb_node_t todo;

    // pointer stack
    rb_node_t *stack[1000];
    int top = -1;

    int i = 0;
    while (i < strlen(str))
    {
        if (str[i] == '(')
        {
            // push the node as being processed
            top ++;
            stack[top] = malloc(sizeof(rb_node_t));
            stack[top]->parent = NULL_TREE_NODE_PTR;
            stack[top]->left = &todo;
            stack[top]->right = &todo;

            // scan the value
            // (value,
            int j = i + 1;
            while ('0' <= str[j] && str[j] <= '9')
            {
                ++ j;
            }
            stack[top]->value = string2uint_range(str, i + 1, j - 1);

            i = j + 1;
            continue;
        }
        else if (str[i] == ')')
        {
            // pop the being processed node
            if (top == 0)
            {
                // pop root
                assert(stack[0]->left != &todo && stack[0]->right != &todo);
                return stack[0];
            }

            rb_node_t *p = stack[top - 1];
            rb_node_t *t = stack[top];
            assert(t->left != &todo && t->right != &todo);

            top --;

            // else, pop this node
            if (p->left == &todo)
            {
                p->left = t;
                p->left->parent = p;
                i ++;
                continue;
            }
            else if (p->right == &todo)
            {
                p->right = t;
                p->right->parent = p;
                i ++;
                continue;
            }

            printf("node %p:%lx is not having any unprocessed sub-tree\n  while %p:%lx is redblack into it.\n",
                p, p->value, t, t->value);
            exit(0);
        }
        else if (str[i] == '#')
        {
            if (top < 0)
            {
                assert(strlen(str) == 1);
                return NULL_TREE_NODE_PTR;
            }

            // push NULL_TREE_NODE_PTR node
            // pop NULL_TREE_NODE_PTR node
            if (stack[top]->left == &todo)
            {
                // must check parent's left node first
                stack[top]->left = NULL_TREE_NODE_PTR;
                i ++;
                continue;
            }
            else if (stack[top]->right == &todo)
            {
                // then check parent's right node
                stack[top]->right = NULL_TREE_NODE_PTR;
                i ++;
                continue;
            }

            printf("node %p:(%lx) is not having any unprocessed sub-tree\n  while NULL_TREE_NODE_PTR is redblack into it.\n",
                stack[top], stack[top]->value);
            exit(0);
        }
        else
        {
            // space, comma, new line
            i ++;
            continue;
        }
    }

    return NULL_TREE_NODE_PTR;
}


#ifdef DEBUG_BST

static int compare_tree(rb_node_t *a, rb_node_t *b)
{
    if (a == NULL_TREE_NODE_PTR && b == NULL_TREE_NODE_PTR)
    {
        return 1;
    }

    if (a == NULL_TREE_NODE_PTR || b == NULL_TREE_NODE_PTR)
    {
        return 0;
    }

    // both not NULL_TREE_NODE_PTR
    if (a->value == b->value)
    {
        return  compare_tree(a->left, b->left) && 
                compare_tree(a->right, b->right);
    }
    else
    {
        return 0;
    }
}

static void test_build()
{
    printf("Testing build tree from string ...\n");

    rb_node_t *r;

    char s[1000];

    memset(s, 0, sizeof(char) * 1000);
    strcpy(s, "#");
    r = tree_construct(s);
    assert(r == NULL_TREE_NODE_PTR);
    tree_free(r);

    memset(s, 0, sizeof(char) * 1000);
    strcpy(s, "(12, #, #)");
    r = tree_construct(s);
    assert(r->parent == NULL_TREE_NODE_PTR);
    assert(r->left == NULL_TREE_NODE_PTR);
    assert(r->right == NULL_TREE_NODE_PTR);
    assert(r->value == 12);
    tree_free(r);

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
    r = tree_construct(s);

    rb_node_t *n1 = r->left->left->left;
    rb_node_t *n2 = r->left->left;
    rb_node_t *n3 = r->left;
    rb_node_t *n4 = r->left->right;
    rb_node_t *n5 = r->left->right->right;
    rb_node_t *n6 = r;
    rb_node_t *n7 = r->right;
    rb_node_t *n8 = r->right->right;

    assert(n1->value == 1);
    assert(n1->parent == n2);
    assert(n1->left == NULL_TREE_NODE_PTR);
    assert(n1->right == NULL_TREE_NODE_PTR);

    assert(n2->value == 2);
    assert(n2->parent == n3);
    assert(n2->left == n1);
    assert(n2->right == NULL_TREE_NODE_PTR);

    assert(n3->value == 3);
    assert(n3->parent == n6);
    assert(n3->left == n2);
    assert(n3->right == n4);

    assert(n4->value == 4);
    assert(n4->parent == n3);
    assert(n4->left == NULL_TREE_NODE_PTR);
    assert(n4->right == n5);

    assert(n5->value == 5);
    assert(n5->parent == n4);
    assert(n5->left == NULL_TREE_NODE_PTR);
    assert(n5->right == NULL_TREE_NODE_PTR);

    assert(n6->value == 6);
    assert(n6->parent == NULL_TREE_NODE_PTR);
    assert(n6->left == n3);
    assert(n6->right == n7);

    assert(n7->value == 7);
    assert(n7->parent == n6);
    assert(n7->left == NULL_TREE_NODE_PTR);
    assert(n7->right == n8);

    assert(n8->value == 8);
    assert(n8->parent == n7);
    assert(n8->left == NULL_TREE_NODE_PTR);
    assert(n8->right == NULL_TREE_NODE_PTR);

    tree_free(r);

    printf("\tPass\n");
}

static void test_delete()
{
    printf("Testing Binary Search tree insertion ...\n");

    rb_node_t *r = tree_construct(
        "(10,"
            "(4,"
                "(2,(1,#,#),(3,#,#)),"
                "(7,"
                    "(6,(5,#,#),#),"
                    "(8,#,(9,#,#))"
                ")"
            "),"
            "(17,"
                "(12,(11,#,#),(13,#,(15,(14,#,#),(16,#,#)))),"
                "(19,(18,#,#),(24,(22,(20,#,(21,#,#)),(23,#,#)),(25,#,#)))"
            ")"
        ")"
    );
    rb_node_t *a;

    // case 1: leaf node - parent's left child
    r = bst_delete(r, r->left->left->left);
    a = tree_construct(
        "(10,"
            "(4,"
                "(2,#,(3,#,#)),"
                "(7,"
                    "(6,(5,#,#),#),"
                    "(8,#,(9,#,#))"
                ")"
            "),"
            "(17,"
                "(12,(11,#,#),(13,#,(15,(14,#,#),(16,#,#)))),"
                "(19,(18,#,#),(24,(22,(20,#,(21,#,#)),(23,#,#)),(25,#,#)))"
            ")"
        ")"
    );
    assert(compare_tree(r, a) == 1);
    tree_free(a);

    // case 2: leaf node - parent's right child
    r = bst_delete(r, r->left->left->right);
    a = tree_construct(
        "(10,"
            "(4,"
                "(2,#,#),"
                "(7,"
                    "(6,(5,#,#),#),"
                    "(8,#,(9,#,#))"
                ")"
            "),"
            "(17,"
                "(12,(11,#,#),(13,#,(15,(14,#,#),(16,#,#)))),"
                "(19,(18,#,#),(24,(22,(20,#,(21,#,#)),(23,#,#)),(25,#,#)))"
            ")"
        ")"
    );
    assert(compare_tree(r, a) == 1);
    tree_free(a);

    // case 3: right NULL_TREE_NODE_PTR
    r = bst_delete(r, r->left->right->left);
    a = tree_construct(
        "(10,"
            "(4,"
                "(2,#,#),"
                "(7,"
                    "(5,#,#),"
                    "(8,#,(9,#,#))"
                ")"
            "),"
            "(17,"
                "(12,(11,#,#),(13,#,(15,(14,#,#),(16,#,#)))),"
                "(19,(18,#,#),(24,(22,(20,#,(21,#,#)),(23,#,#)),(25,#,#)))"
            ")"
        ")"
    );
    assert(compare_tree(r, a) == 1);
    tree_free(a);

    // case 4: left NULL_TREE_NODE_PTR
    r = bst_delete(r, r->left->right->right);
    a = tree_construct(
        "(10,"
            "(4,"
                "(2,#,#),"
                "(7,"
                    "(5,#,#),"
                    "(9,#,#)"
                ")"
            "),"
            "(17,"
                "(12,(11,#,#),(13,#,(15,(14,#,#),(16,#,#)))),"
                "(19,(18,#,#),(24,(22,(20,#,(21,#,#)),(23,#,#)),(25,#,#)))"
            ")"
        ")"
    );
    assert(compare_tree(r, a) == 1);
    tree_free(a);

    // case 5: right child's left NULL_TREE_NODE_PTR
    r = bst_delete(r, r->right->left);
    a = tree_construct(
        "(10,"
            "(4,"
                "(2,#,#),"
                "(7,"
                    "(5,#,#),"
                    "(9,#,#)"
                ")"
            "),"
            "(17,"
                "(13,(11,#,#),(15,(14,#,#),(16,#,#))),"
                "(19,(18,#,#),(24,(22,(20,#,(21,#,#)),(23,#,#)),(25,#,#)))"
            ")"
        ")"
    );
    assert(compare_tree(r, a) == 1);
    tree_free(a);

    // case 6: right child's left not NULL_TREE_NODE_PTR
    r = bst_delete(r, r->right->right);
    a = tree_construct(
        "(10,"
            "(4,"
                "(2,#,#),"
                "(7,"
                    "(5,#,#),"
                    "(9,#,#)"
                ")"
            "),"
            "(17,"
                "(13,(11,#,#),(15,(14,#,#),(16,#,#))),"
                "(20,(18,#,#),(24,(22,(21,#,#),(23,#,#)),(25,#,#)))"
            ")"
        ")");
    assert(compare_tree(r, a) == 1);
    tree_free(a);

    // case 7: delete root
    r = bst_delete(r, r);
    a = tree_construct(
        "(11,"
            "(4,"
                "(2,#,#),"
                "(7,"
                    "(5,#,#),"
                    "(9,#,#)"
                ")"
            "),"
            "(17,"
                "(13,#,(15,(14,#,#),(16,#,#))),"
                "(20,(18,#,#),(24,(22,(21,#,#),(23,#,#)),(25,#,#)))"
            ")"
        ")");
    int equal = compare_tree(r, a);
    assert(compare_tree(r, a) == 1);
    tree_free(a);

    tree_free(r);

    // case 8: delete a NULL_TREE_NODE_PTR tree
    r = NULL_TREE_NODE_PTR;
    r = bst_delete(r, r);
    assert(r == NULL_TREE_NODE_PTR);

    printf("\tPass\n");
}

static void test_insert()
{
    printf("Testing Binary Search tree insertion ...\n");

    rb_node_t *r = tree_construct(
        "(11,"
            "(2,(1,#,#),(7,(5,#,#),(8,#,#))),"
            "(14,#,(15,#,#)))"
    );

    // test insert
    r = bst_insert(r, 4);

    // check
    rb_node_t *a = tree_construct(
        "(11,"
            "(2,(1,#,#),(7,(5,(4,#,#),#),(8,#,#))),"
            "(14,#,(15,#,#)))"
    );
    assert(compare_tree(r, a) == 1);

    // free
    tree_free(r);
    tree_free(a);

    printf("\tPass\n");
}

int main()
{
    test_build();
    test_insert();
    test_delete();
}

#endif
