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

int rbt_compare(rb_tree_t *a, rb_tree_t *b);
rb_tree_t *rbt_construct_keystr(char *tree, char *color);
rb_tree_t *bst_construct_keystr(char *str);
void rbt_rotate(rb_node_t *n, rb_node_t *p, rb_node_t *g, rb_tree_t *tree);
void rbt_verify(rb_tree_t *tree);

int bst_compare(rb_tree_t *a, rb_tree_t *b);
void bst_print(rb_node_t *node);

static void test_delete()
{
    printf("Testing Red-Black tree deletion ...\n");

    rb_tree_t *r;
    rb_tree_t *a;

    // no double black
    // bst case 2 - single child
    r = rbt_construct_keystr(
        "(10,"
            "(5,#,(9,#,#)),"
            "(15,#,#)"
        ")",
        "BB#R##B##");
    rbt_remove(r, 5);
    a = rbt_construct_keystr(
        "(10,"
            "(9,#,#),"
            "(15,#,#)"
        ")",
        "BB##B##");
    assert(rbt_compare(r, a) == 1);
    bst_free(r);
    bst_free(a);

    // no double black
    // bst case 2 - single child
    r = rbt_construct_keystr(
        "(10,"
            "(5,(9,#,#),#),"
            "(15,#,#)"
        ")",
        "BBR###B##");
    rbt_remove(r, 5);
    a = rbt_construct_keystr(
        "(10,"
            "(9,#,#),"
            "(15,#,#)"
        ")",
        "BB##B##");
    assert(rbt_compare(r, a) == 1);
    bst_free(r);
    bst_free(a);

    // no double black
    // bst case 3.1 - 2 childs
    // x->right->left == NULL
    // (R, T1, (B, #, (R, #, #)))
    r = rbt_construct_keystr(
        "(10,"
            "(5,"   // delete - red
                "(2,#,#),"    // T1
                "(6,#,(7,#,#))"  // successor
            "),"
            "(15,#,#)"  // T1
        ")",
        "BRB##B#R##B##");
    rbt_remove(r, 5);
    a = rbt_construct_keystr(
        "(10,"
            "(6,"   // T1
                "(2,#,#),"    // T1
                "(7,#,#)"
            "),"
            "(15,#,#)"  // T1
        ")",
        "BRB##B##B##");
    assert(rbt_compare(r, a) == 1);
    bst_free(r);
    bst_free(a);

    // no double black
    // bst case 3.1 - 2 childs
    // x->right->left == NULL
    // (B, T1, (B, #, (R, #, #)))
    r = rbt_construct_keystr(
        "(10,"
            "(5,"   // delete - black
                "(2,#,#),"    // T1
                "(6,#,(7,#,#))"  // successor
            "),"
            "(15,(12,#,#),(16,#,#))"  // T2
        ")",
        "BBB##B#R##BB##B##");
    rbt_remove(r, 5);
    a = rbt_construct_keystr(
        "(10,"
            "(6,"   // T2
                "(2,#,#),"    // T1
                "(7,#,#)"
            "),"
            "(15,(12,#,#),(16,#,#))"  // T2
        ")",
        "BBB##B##BB##B##");
    assert(rbt_compare(r, a) == 1);
    bst_free(r);
    bst_free(a);

    // no double black
    // bst case 3.1 - 2 childs
    // (B, T0, (R, #, #))
    r = rbt_construct_keystr(
        "(10,"
            "(5,"   // delete - black
                "(2,#,#),"    // T0
                "(7,#,#)"  // successor
            "),"
            "(15,#,#)"  // T1
        ")",
        "BBR##R##B##");
    rbt_remove(r, 5);
    a = rbt_construct_keystr(
        "(10,"
            "(7,"   // delete - black
                "(2,#,#),"    // T0
                "#"
            "),"
            "(15,#,#)"  // T1
        ")",
        "BBR###B##");
    assert(rbt_compare(r, a) == 1);
    bst_free(r);
    bst_free(a);

    // no double black
    // bst case 3.2 - 2 childs
    // successor (R, #, #)
    r = rbt_construct_keystr(
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
    rbt_remove(r, 6);
    a = rbt_construct_keystr(
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
    assert(rbt_compare(r, a) == 1);
    bst_free(r);
    bst_free(a);

    // no double black
    // bst case 3.2 - 2 childs
    // successor (B, #, (R, #, #)), parent red
    r = rbt_construct_keystr(
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
    rbt_remove(r, 12);
    a = rbt_construct_keystr(
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
    assert(rbt_compare(r, a) == 1);
    bst_free(r);
    bst_free(a);

    // no double black
    // bst case 3.2 - 2 childs
    // successor (B, #, (R, #, #)), parent black
    r = rbt_construct_keystr(
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
    rbt_remove(r, 12);
    a = rbt_construct_keystr(
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
    assert(rbt_compare(r, a) == 1);
    bst_free(r);
    bst_free(a);

    // delete red node
    r = rbt_construct_keystr(
        "(10,"
            "(5,(2,#,#),(9,#,#)),"
            "(30,(25,#,#),(40,(38,#,#),#))"
        ")",
        "BRB##B##RB##BR###");
    rbt_remove(r, 38);
    a = rbt_construct_keystr(
        "(10,"
            "(5,(2,#,#),(9,#,#)),"
            "(30,(25,#,#),(40,#,#))"
        ")",
        "BRB##B##RB##B##");
    assert(rbt_compare(r, a) == 1);
    bst_free(r);
    bst_free(a);

    // delete black node - simple
    r = rbt_construct_keystr(
        "(10,"
            "(5,(2,#,#),(9,#,#)),"
            "(30,(25,#,#),(40,(35,#,(38,#,#)),(50,#,#)))"
        ")",
        "BBB##B##BB##RB#R##B##");
    rbt_remove(r, 30);
    a = rbt_construct_keystr(
        "(10,"
            "(5,(2,#,#),(9,#,#)),"
            "(35,(25,#,#),(40,(38,#,#),(50,#,#)))"
        ")",
        "BBB##B##BB##RB##B##");
    assert(rbt_compare(r, a) == 1);
    bst_free(r);
    bst_free(a);

    // delete a double black node
    // double black = 15
    //  1. sibling black = 30
    //  2. both sibling's childs black = (NULL, NULL)
    // double black gives black to parent
    // parent red, then black
    // sibling red
    r = rbt_construct_keystr(
        "(10,"
            "(5,#,#),"
            "(20,(15,#,#),(30,#,#))"
        ")",
        "BB##RB##B##");
    rbt_remove(r, 15);
    a = rbt_construct_keystr(
        "(10,"
            "(5,#,#),"
            "(20,#,(30,#,#))"
        ")",
        "BB##B#R##");
    assert(rbt_compare(r, a) == 1);
    bst_free(r);
    bst_free(a);

    // delete a double black node
    // double black = 15
    //  1. sibling black = 30
    //  2. both sibling's childs black = (NULL, NULL)
    // double black gives black to parent
    // parent black, then parent double black, continue to parent, untill root
    // silbing red
    r = rbt_construct_keystr(
        "(10,"
            "(5,(1,#,#),(7,#,#)),"
            "(20,(15,#,#),(30,#,#))"
        ")",
        "BBB##B##BB##B##");
    rbt_remove(r, 15);
    a = rbt_construct_keystr(
        "(10,"
            "(5,(1,#,#),(7,#,#)),"
            "(20,#,(30,#,#))"
        ")",
        "BRB##B##B#R##");
    assert(rbt_compare(r, a) == 1);
    bst_free(r);
    bst_free(a);

    // delete a double black node
    // double black = 15
    //  1. sibling **RED** = 30
    //  2. both sibling's childs black = (NULL, NULL)
    // double black gives black to parent then to sibling
    // parent black, sibling red ==> parent red, sibling black
    r = rbt_construct_keystr(
        "(10,"
            "(5,(1,#,#),(7,#,#)),"
            "(20,(15,#,#),(30,(25,#,#),(40,#,#)))"
        ")",
        "BBB##B##BB##RB##B##");
    rbt_remove(r, 15);
    a = rbt_construct_keystr(
        "(10,"
            "(5,(1,#,#),(7,#,#)),"
            "(30,(20,#,(25,#,#)),(40,#,#))"
        ")",
        "BBB##B##BB#R##B##");
    assert(rbt_compare(r, a) == 1);
    bst_free(r);
    bst_free(a);

    // delete
    // sibling black, far child red, near child black
    // sibling black, far child black, near child red
    r = rbt_construct_keystr(
        "(10,"
            "(5,(1,#,#),(7,#,#)),"
            "(30,(25,(20,#,#),(28,#,#)),(40,#,#))"
        ")",
        "BBB##B##BRB##B##B##");
    rbt_remove(r, 1);
    a = rbt_construct_keystr(
        "(25,"
            "(10,(5,#,(7,#,#)),(20,#,#)),"
            "(30,(28,#,#),(40,#,#))"
        ")",
        "BBB#R##B##BB##B##");
    assert(rbt_compare(r, a) == 1);
    bst_free(r);
    bst_free(a);

    // A COMPLETE TEST CASE

    // silbing red
    r = rbt_construct_keystr(
        "(50,"
            "(20,(15,#,#),(35,#,#)),"
            "(65,"
                "(55,#,#),"
                "(70,(68,#,#),(80,#,(90,#,#)))"
            ")"
        ")",
        "BBB##B##BB##RB##B#R##");

    // delete 55 - sibling's 2 black
    rbt_remove(r, 55);
    a = rbt_construct_keystr(
        "(50,"
            "(20,(15,#,#),(35,#,#)),"
            "(70,"
                "(65,#,(68,#,#)),"
                "(80,#,(90,#,#))"
            ")"
        ")",
        "BBB##B##BB#R##B#R##");
    assert(rbt_compare(r, a) == 1);
    bst_free(a);

    // delete 20 - root double black
    assert(((rb_node_t *)(r->root))->left->key == 20);
    rbt_remove(r, 20);
    a = rbt_construct_keystr(
        "(50,"
            "(35,(15,#,#),#),"
            "(70,"
                "(65,#,(68,#,#)),"
                "(80,#,(90,#,#))"
            ")"
        ")",
        "BBR###RB#R##B#R##");
    assert(rbt_compare(r, a) == 1);
    bst_free(a);
    
    // delete 90 - red node
    assert(((rb_node_t *)(r->root))->right->right->right->key == 90);
    rbt_remove(r, 90);
    a = rbt_construct_keystr(
        "(50,"
            "(35,(15,#,#),#),"
            "(70,"
                "(65,#,(68,#,#)),"
                "(80,#,#)"
            ")"
        ")",
        "BBR###RB#R##B##");
    assert(rbt_compare(r, a) == 1);
    bst_free(a);
    
    // delete 80 - sibling black, near child red, far child black
    assert(((rb_node_t *)(r->root))->right->right->key == 80);
    rbt_remove(r, 80);
    a = rbt_construct_keystr(
        "(50,"
            "(35,(15,#,#),#),"
            "(68,"
                "(65,#,#),"
                "(70,#,#)"
            ")"
        ")",
        "BBR###RB##B##");
    assert(rbt_compare(r, a) == 1);
    bst_free(a);
    
    // delete 50 - root, and having 1B 1R childs, no parent nor sibling
    assert(((rb_node_t *)(r->root))->key == 50);
    rbt_remove(r, 50);
    a = rbt_construct_keystr(
        "(65,"
            "(35,(15,#,#),#),"
            "(68,#,(70,#,#))"
            ")"
        ")",
        "BBR###B#R##");
    assert(rbt_compare(r, a) == 1);
    bst_free(a);
    
    // delete 35 - having red child
    assert(((rb_node_t *)(r->root))->left->key == 35);
    rbt_remove(r, 35);
    a = rbt_construct_keystr(
        "(65,"
            "(15,#,#),"
            "(68,#,(70,#,#))"
            ")"
        ")",
        "BB##B#R##");
    assert(rbt_compare(r, a) == 1);
    bst_free(a);
    
    // delete 15 - far child red sibling black
    assert(((rb_node_t *)(r->root))->left->key == 15);
    rbt_remove(r, 15);
    a = rbt_construct_keystr(
        "(68,(65,#,#),(70,#,#))",
        "BB##B##");
    assert(rbt_compare(r, a) == 1);
    bst_free(a);
    
    // delete 65 - both s-childs black
    assert(((rb_node_t *)(r->root))->left->key == 65);
    rbt_remove(r, 65);
    a = rbt_construct_keystr(
        "(68,#,(70,#,#))",
        "B#R##");
    assert(rbt_compare(r, a) == 1);
    bst_free(a);

    bst_free(r);

    printf("\033[32;1m\tPass\033[0m\n");
}

static void test_insert()
{
    printf("Testing Red-Black tree insertion ...\n");

    rb_tree_t *r = rbt_construct_keystr(
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
    rbt_verify(r);
    rbt_add(r, 4);

    // check
    rb_tree_t *ans = rbt_construct_keystr(
        "(5,(2,(1,#,#),(4,#,#)),(11,(7,#,(8,#,#)),(14,#,(15,#,#))))",
        "B"
            "B"
                "B##"
                "B##"
            "B"
                "B#R##"
                "B#R##");

    rbt_verify(ans);
    rbt_verify(r);

    assert(rbt_compare(r, ans) == 1);

    // randomly insert values
    int loops = 50000;
    int iteration = 1000;

    for (int i = 0; i < loops; ++ i)
    {
        if (i % iteration == 0)
        {
            printf("insert %d / %d\n", i, loops);
        }

        uint64_t key = rand() % 1000000;
        rbt_add(r, key);
        rbt_verify(r);
    }

    bst_free(r);
    bst_free(ans);

    printf("\033[32;1m\tPass\033[0m\n");
}

static void test_rotate()
{
    printf("Testing Red-Black tree rotation ...\n");

    rb_tree_t *t;
    rb_tree_t *a;

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

    char rotated[100] = "(4,(2,(1,#,#),(3,#,#)),(6,(5,#,#),(7,#,#)))";
    char rotated2[100] = "(0,#,(4,(2,(1,#,#),(3,#,#)),(6,(5,#,#),(7,#,#))))";

    rb_node_t *g = NULL;
    rb_node_t *p = NULL;
    rb_node_t *n = NULL;

    for (int i = 0; i < 8; ++ i)
    {
        t = bst_construct_keystr(inputs[i]);

        if ((0x1 & (i >> 2)) == 0)
        {
            // test grandparent root
            g = (rb_node_t *)t->root;
        }
        else
        {
            // test grandparent not root
            g = ((rb_node_t *)(t->root))->right;
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

        rbt_rotate(n, p, g, t);
        if (i < 4)
        {
            a = bst_construct_keystr(rotated);
        }
        else
        {
            a = bst_construct_keystr(rotated2);
        }
        assert(bst_compare(t, a) == 1);

        bst_free(a);
        bst_free(t);
    }
    
    printf("\033[32;1m\tPass\033[0m\n");
}

static void test_insert_delete()
{
    printf("Testing Red-Black Tree insertion and deletion ...\n");

    rb_tree_t *tree = rbt_construct();
    
    // insert
    int loops = 50000;
    int iteration = 1000;

    uint64_t *array = malloc(loops * sizeof(uint64_t));
    for (int i = 0; i < loops; ++ i)
    {
        if (i % iteration == 0)
        {
            printf("insert %d / %d\n", i, loops);
        }
        uint64_t key = rand() % 1000000;
        rbt_add(tree, key);
        rbt_verify(tree);
        array[i] = key;
    }

    // mark
    for (int i = 0; i < loops; ++ i)
    {
        if (i % iteration == 0)
        {
            printf("delete %d / %d\n", i, loops);
        }

        int index = rand() % loops;

        rbt_remove(tree, array[index]);
        rbt_verify(tree);
        array[index] = 0xFFFFFFFFFFFFFFFF;
    }

    // sweep
    for (int i = 0; i < loops; ++ i)
    {
        if (array[i] != 0xFFFFFFFFFFFFFFFF)
        {
            rbt_remove(tree, array[i]);
            rbt_verify(tree);
        }
    }

    assert(tree->root == 0);

    free(array);
    rbt_free(tree);

    printf("\033[32;1m\tPass\033[0m\n");
}

int main()
{
    srand(123456);

    //test_rotate();
    //test_delete();
    //test_insert();
    test_insert_delete();
}