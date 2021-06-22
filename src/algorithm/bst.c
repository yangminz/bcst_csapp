#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <headers/algorithm.h>
#include <headers/common.h>

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
    if (root == NULL)
    {
        root = malloc(sizeof(rb_node_t));
        
        // update properties
        root->parent = NULL;
        root->left = NULL;
        root->right = NULL;
        root->value = val;
        root->color = COLOR_BLACK;

        *redblack = root;
        return root;
    }

    // search the right place (leaf node) to insert data
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
            if (n->right == NULL)
            {
                // insert here
                n->right = malloc(sizeof(rb_node_t));
                n->right->parent = n;
                n->right->left = NULL;
                n->right->right = NULL;
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

    *redblack = NULL;
    return root;
}

rb_node_t *bst_delete(rb_node_t *root, uint64_t val)
{
    rb_node_t *redblack;
    rb_node_t *n = bst_find(root, val);
    return bst_delete_node(root, n, &redblack);
}

// delete the node
// return the updated tree root node
// this delete will try to fix the RBT with recoloring
// and only double black NULL nodes need further fixing
// record the parent node of this double black node
rb_node_t *bst_delete_node(rb_node_t *root, rb_node_t *n, rb_node_t **redblack)
{
    *redblack = NULL;
    if (n == NULL)
    {
        return NULL;
    }

    // in case root is deleted
    rb_node_t *p = NULL;
    int is_n_root = 0;
    if (n == root)
    {
        // no parent pointer, create a dummy one
        p = malloc(sizeof(rb_node_t));
        p->color = COLOR_BLACK;
        p->left = n;
        n->parent = p;
        p->parent = NULL;
        p->right = NULL;
        is_n_root = 1;
    }
    else
    {
        p = n->parent;
        is_n_root = 0;
    }

    int p_n_index = (n == n->parent->right);

    if (n->left == NULL && n->right == NULL)
    {
        //////////////////////////////////////////////
        // case 1: leaf node                        //
        //////////////////////////////////////////////
        n->parent->childs[p_n_index] = NULL;
        
        if (is_n_root == 1)
        {
            // tree is empty now, no steps
            root = NULL;
        }
        else if (n->color == COLOR_BLACK)
        {
            // for RB, we record the parent node of double black
            *redblack = p;
        }

        // when this leaf node is black, DOUBLE BLACK for NULL
        free(n);
    }
    else if (n->left == NULL || n->right == NULL)
    {
        //////////////////////////////////////////////
        // case 2: one sub-tree is empty            //
        //////////////////////////////////////////////

        // transplant the not-empty sub-tree to the node to be deleted
        // 0 - left; 1 - right
        rb_node_t *c = n->childs[n->left == NULL];

        /*  With one child is NULL, there are only 2 possibilities:
            CASE 1 - this node is red, then its black height is 0
                T0 --> # | (R, #, #)
                impossible
            CASE 2 - this node is black, then its black height is 1
                T1 --> (B, #, T0) | (B, T0, #)

                (B, #, (R, #, #))
                (B, (R, #, #), #)
                pass the black to the child, then done:
                (B, #, #)
         */
#ifdef DEBUG_REDBLACK
        assert(n->color == COLOR_BLACK);
        assert(c->color == COLOR_RED);
        assert(c->left == NULL);
        assert(c->right == NULL);
#endif
        
        // we do not manipulate the pointers here so we can remain the colors of rbt
        n->value = c->value;
        n->left = c->left;
        n->right = c->right;
        n->color = COLOR_BLACK; // should be still black

        // this happens in BST only
        // for RBT, they are all NULL
        if (n->left != NULL)
        {
            n->left->parent = n;
        }

        if (n->right != NULL)
        {
            n->right->parent = n;
        }
        
        free(c);
    }
    else if (n->right->left == NULL)
    {
        //////////////////////////////////////////////
        // case 3: both sub-trees are not empty     //
        // 3.1: a simple remove will do the job     //
        //////////////////////////////////////////////

        // transplant the left sub-tree
        rb_node_t *successor = n->right;

        /*  successor is at most height 1: (successor, #, T0)
            CASE 1 - successor red
                (R, #, T0) --> (R, #, #), just delete it
            CASE 2 - successor black
                (B, #, T0) --> 
                    (B, #, #)   NULL double black
                    (B, #, (R, #, #))
                        With successor (B, #, (R, #, #)), the node to be deleted n can be

                        CASE 1 - n red
                            T1 = (R, T1, (B, #, (R, #, #)))
                            after deletion, (B, T1, (R, #, #))
                            recoloring, (R, T1, (B, #, #))
                        CASE 2 - n black
                            T2 = (B, T1, (B, #, (R, #, #)))
                            after deletion, (DB, T1, (R, #, #))
                            recoloring, (B, T1, (B, #, #))
         */
#ifdef DEBUG_REDBLACK
        if (successor->color == COLOR_RED)
        {
            assert(successor->right == NULL);
        }
        else if (successor->right != NULL)
        {
            assert(successor->right->color == COLOR_RED);
            assert(successor->right->left == NULL);
            assert(successor->right->right == NULL);
        }
#endif
        if (successor->color == COLOR_BLACK && successor->right == NULL)
        {
            // successor (B, #, #)
            *redblack = n;
        }
        
        n->value = successor->value;

        n->right = successor->right;
        if (n->right != NULL)
        {
            n->right->parent = n;
            // for red-black tree
            // (B, #, (R, #, #)) --> (R, #, #) --> (B, #, #)
            assert(n->right->color == COLOR_RED);
            n->right->color = COLOR_BLACK;
        }

        free(successor);
    }
    else if (n->right->left != NULL)
    {
        //////////////////////////////////////////////
        // 3.2: float up the tight upper bound      //
        // as root of sub-tree                      //
        //////////////////////////////////////////////

        rb_node_t *successor = n->right;
        while (successor->left != NULL)
        {
            successor = successor->left;
        }

        // float up successor
        n->value = successor->value;
        
        /*  Same analysis, for (Successor, #, T0)
                (R, #, T0) -->
                    (R, #, #), just delete it

                (B, #, T0) -->
                    (B, #, #), NULL becomes double black
                    (B, #, (R, #, #)), consider the parent:
                        (R, (B, #, (R, #, #)), T1)
                            free successor: (R, (R, #, #), T1); recoloring: (R, (B, #, #), T1)
                        (B, (B, #, (R, #, #)), T1)
                            free successor: (B, (R, #, #), T1); recoloring: (B, (B, #, #), T1)
         */

        successor->parent->left = successor->right;
        if (successor->right != NULL)
        {
            successor->right->parent = successor->parent;
            // for RBT, only (B, #, (R, #, #))
            // a simple recoloring will fix
            successor->right->color = COLOR_BLACK;
        }
        else if (successor->color == COLOR_BLACK)
        {
            // for RBT, only (B, #, #)
            // and successor should not be root
            *redblack = successor->parent;
        }

        // when this successor is black, DOUBLE BLACK for successor
        free(successor);
    }
    
    if (is_n_root == 1)
    {
        free(p);

        if (root != NULL)
        {
            root->parent = NULL;
        }
    }

    return root;
}

// find the node owning the target value
rb_node_t *bst_find(rb_node_t *root, uint64_t val)
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

void tree_free(rb_node_t *root)
{
    if (root == NULL)
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
    // for NULL node, #

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
            stack[top]->parent = NULL;
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
                return NULL;
            }

            // push NULL node
            // pop NULL node
            if (stack[top]->left == &todo)
            {
                // must check parent's left node first
                stack[top]->left = NULL;
                i ++;
                continue;
            }
            else if (stack[top]->right == &todo)
            {
                // then check parent's right node
                stack[top]->right = NULL;
                i ++;
                continue;
            }

            printf("node %p:(%lx) is not having any unprocessed sub-tree\n  while NULL is redblack into it.\n",
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

    return NULL;
}

static void tree_print_dfs(rb_node_t *root)
{
    if (root == NULL)
    {
        printf("#");
        return;
    }

    if (root->color == COLOR_RED)
    {
        printf("(\033[31m%lu\033[0m,", root->value);
    }
    else
    {
        printf("(%lu,", root->value);
    }

    tree_print_dfs(root->left);
    printf(",");
    tree_print_dfs(root->right);
    printf(")");
}

void tree_print(rb_node_t *root)
{
    tree_print_dfs(root);
    printf("\n");
}

#ifdef DEBUG_BST

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
    assert(r == NULL);
    tree_free(r);

    memset(s, 0, sizeof(char) * 1000);
    strcpy(s, "(12, #, #)");
    r = tree_construct(s);
    assert(r->parent == NULL);
    assert(r->left == NULL);
    assert(r->right == NULL);
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
    assert(n1->left == NULL);
    assert(n1->right == NULL);

    assert(n2->value == 2);
    assert(n2->parent == n3);
    assert(n2->left == n1);
    assert(n2->right == NULL);

    assert(n3->value == 3);
    assert(n3->parent == n6);
    assert(n3->left == n2);
    assert(n3->right == n4);

    assert(n4->value == 4);
    assert(n4->parent == n3);
    assert(n4->left == NULL);
    assert(n4->right == n5);

    assert(n5->value == 5);
    assert(n5->parent == n4);
    assert(n5->left == NULL);
    assert(n5->right == NULL);

    assert(n6->value == 6);
    assert(n6->parent == NULL);
    assert(n6->left == n3);
    assert(n6->right == n7);

    assert(n7->value == 7);
    assert(n7->parent == n6);
    assert(n7->left == NULL);
    assert(n7->right == n8);

    assert(n8->value == 8);
    assert(n8->parent == n7);
    assert(n8->left == NULL);
    assert(n8->right == NULL);

    tree_free(r);

    printf("\033[32;1m\tPass\033[0m\n");
}

static void test_delete()
{
    printf("Testing Binary Search tree deletion ...\n");

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
    r = bst_delete(r, 1);
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
    r = bst_delete(r, 3);
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

    // case 3: right NULL
    r = bst_delete(r, 6);
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

    // case 4: left NULL
    r = bst_delete(r, 8);
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

    // case 5: right child's left NULL
    r = bst_delete(r, 12);
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

    // case 6: right child's left not NULL
    r = bst_delete(r, 19);
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
    r = bst_delete(r, 10);
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

    printf("\033[32;1m\tPass\033[0m\n");
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
    printf("\033[32;1m\tPass\033[0m\n");
}

int main()
{
    test_build();
    test_insert();
    test_delete();
}

#endif
