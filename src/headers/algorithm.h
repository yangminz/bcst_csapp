/* BCST - Introduction to Computer Systems
 * Author:      yangminz@outlook.com
 * Github:      https://github.com/yangminz/bcst_csapp
 * Bilibili:    https://space.bilibili.com/4564101
 * Zhihu:       https://www.zhihu.com/people/zhao-yang-min
 * This project (code repository and videos) is exclusively owned by yangminz 
 * and shall not be used for commercial and profitting purpose 
 * without yangminz's permission.
 */

// include guards to prevent double declaration of any identifiers 
// such as types, enums and static variables
#ifndef DATASTRUCTURE_GUARD
#define DATASTRUCTURE_GUARD

#include <stdint.h>

#define NULL_ID (0)

/*======================================*/
/*      Dynamic Array                   */
/*======================================*/
typedef struct
{
    uint32_t size;
    uint32_t count;
    uint64_t *table;
} array_t;

array_t *array_construct(int size);
void array_free(array_t *arr);
array_t *array_insert(array_t *arr, uint64_t value);
int array_delete(array_t *arr, int index);
int array_get(array_t *arr, int index, uint64_t *valptr);

/*======================================*/
/*      Extendible Hash Table           */
/*======================================*/
typedef struct
{
    int localdepth;     // the local depth
    int counter;        // the counter of slots (have data)
    char **karray;
    uint64_t *varray;
} hashtable_bucket_t;

typedef struct
{
    int num;            // number of buckets = 1 << globaldepth
    int globaldepth;    // the global depth

    int size;           // the size of (key, value) tuples of each bucket
    hashtable_bucket_t **directory;    // the internal table is actually an array
} hashtable_t;

hashtable_t *hashtable_construct(int size);
void hashtable_free(hashtable_t *tab);
int hashtable_get(hashtable_t *tab, char *key, uint64_t *valptr);
hashtable_t *hashtable_insert(hashtable_t *tab, char *key, uint64_t val);

/*======================================*/
/*      Trie - Prefix Tree              */
/*======================================*/
typedef struct TRIE_NODE_STRUCT
{
    hashtable_t *next;
    uint64_t value;
    int isvalue;
} trie_node_t;

trie_node_t * trie_construct();
void trie_free(trie_node_t *root);
trie_node_t *trie_insert(trie_node_t *root, char *key, uint64_t value);
int trie_get(trie_node_t *root, char *key, uint64_t *valptr);
trie_node_t *trie_next(trie_node_t *current, char input);

//  The following data structures and algorithms
//  are designed to be Generic. To use, the user
//  should design their own data structure and
//  implement the interface with function pointers.
//  With this design, the actual data structure
//  can be very free. It can be malloced on heap.
//  And it can be a byte-block like in ./src/malloc

/*======================================*/
/*      Circular Doubly Linked List     */
/*======================================*/

// Define the interface for generic linked list node structure
typedef struct
{
    // "malloc" the memory of a node
    uint64_t (*construct_node)();
    // "free" the memory of a node
    int (*destruct_node)(uint64_t);

    // <uint64_t> "node": the id of node
    // return <uint64_t>: 1 if node is null
    int (*is_null_node)(uint64_t);

    // <uint64_t> "first": the id of first node
    // <uint64_t> "second": the id of second node
    // return <uint64_t>: 0 if they are the same
    int (*compare_nodes)(uint64_t, uint64_t);

    // <uint64_t> "node": the id of current node
    // return <uint64_t>: the id of the previous node
    uint64_t (*get_node_prev)(uint64_t);
    // <uint64_t> "node": the id of current node
    // <uint64_t> "prev": the id of previous node
    // return <int>: 1 if the setting is successful
    int (*set_node_prev)(uint64_t, uint64_t);

    // <uint64_t> "node": the id of current node
    // return <uint64_t>: the id of the next node
    uint64_t (*get_node_next)(uint64_t);
    // <uint64_t> "node": the id of current node
    // <uint64_t> "next": the id of next node
    // return <int>: 1 if the setting is successful
    int (*set_node_next)(uint64_t, uint64_t);

    // <uint64_t> "node": the id of current node
    // return <uint64_t>: the value of node
    uint64_t (*get_node_value)(uint64_t);
    // <uint64_t> "node": the id of current node
    // <uint64_t> "value": the value of the node
    // return <int>: 1 if the setting is successful
    int (*set_node_value)(uint64_t, uint64_t);
} linkedlist_node_interface;

#define ILLIST_CONSTRUCT (1 << 0)
#define ILLIST_DESTRUCT (1 << 1)
#define ILLIST_CHECKNULL (1 << 2)
#define ILLIST_COMPARE (1 << 3)
#define ILLIST_PREV (1 << 4)
#define ILLIST_NEXT (1 << 5)
#define ILLIST_VALUE (1 << 6)
void linkedlist_validate_interface(linkedlist_node_interface *i_node,
    uint64_t flags);

// internal class of the linked list
typedef struct LINKEDLIST_INTERNAL_STRUCT
{
    uint64_t    head;
    int64_t     count;

    // this: this pointer
    // <uint64_t> "node": the id of new head node
    // return <int>: 1 if the updating is successful
    int (*update_head)(struct LINKEDLIST_INTERNAL_STRUCT *this, uint64_t);
} linkedlist_internal_t;

// The linked list implementation open to other data structures
// especially useful for malloc explicit list implementation
int linkedlist_internal_add(linkedlist_internal_t *list, 
    linkedlist_node_interface *i_node, 
    uint64_t value);
int linkedlist_internal_insert(linkedlist_internal_t *list, 
    linkedlist_node_interface *i_node, 
    uint64_t node);
int linkedlist_internal_delete(linkedlist_internal_t *list, 
    linkedlist_node_interface *i_node, 
    uint64_t node);
uint64_t linkedlist_internal_index(linkedlist_internal_t *list,
    linkedlist_node_interface *i_node, 
    uint64_t index);
uint64_t linkedlist_internal_next(linkedlist_internal_t *list,
    linkedlist_node_interface *i_node);
int linkedlist_internal_insert_after(linkedlist_internal_t *list, 
    linkedlist_node_interface *i_node, 
    uint64_t prev,
    uint64_t node);
int linkedlist_internal_insert_before(linkedlist_internal_t *list, 
    linkedlist_node_interface *i_node, 
    uint64_t next,
    uint64_t node);

//
//  The implementation of the default linked list
//

typedef union
{
    linkedlist_internal_t base;
    struct
    {
        uint64_t head;
        int64_t  count;
    };
} linkedlist_t;

typedef struct LINKED_LIST_NODE_STRUCT
{
    uint64_t value;
    struct LINKED_LIST_NODE_STRUCT *prev;
    struct LINKED_LIST_NODE_STRUCT *next;
} linkedlist_node_t;

linkedlist_t *linkedlist_construct();
void linkedlist_free(linkedlist_t *list);
void linkedlist_add(linkedlist_t *list, uint64_t value);
void linkedlist_delete(linkedlist_t *list, linkedlist_node_t *node);
linkedlist_node_t *linkedlist_next(linkedlist_t *list);
linkedlist_node_t *linkedlist_index(linkedlist_t *list, uint64_t index);

/*======================================*/
/*      Red Black Tree                  */
/*======================================*/
typedef enum
{
    COLOR_BLACK,
    COLOR_RED,
} rb_color_t;

// Define the generic class for RB Tree node
typedef struct
{
    // "malloc" the memory of a node
    uint64_t (*construct_node)();
    // "free" the memory of a node
    int (*destruct_node)(uint64_t);

    // <uint64_t> "node": the id of node
    // return <uint64_t>: 1 if node is null
    int (*is_null_node)(uint64_t);

    // <uint64_t> "first": the id of first node
    // <uint64_t> "second": the id of second node
    // return <uint64_t>: 0 if they are the same
    int (*compare_nodes)(uint64_t, uint64_t);

    // <uint64_t> "node": the id of current node
    // return <uint64_t>: the id of the parent node
    uint64_t (*get_parent)(uint64_t);
    // <uint64_t> "node": the id of current node
    // <uint64_t> "parent": the id of parent node
    // return <int>: 1 if the setting is successful
    int (*set_parent)(uint64_t, uint64_t);

    // <uint64_t> "node": the id of current node
    // return <uint64_t>: the id of the left child
    uint64_t (*get_leftchild)(uint64_t);
    // <uint64_t> "node": the id of current node
    // <uint64_t> "left": the id of left child
    // return <int>: 1 if the setting is successful
    int (*set_leftchild)(uint64_t, uint64_t);

    // <uint64_t> "node": the id of current node
    // return <uint64_t>: the right child of node
    uint64_t (*get_rightchild)(uint64_t);
    // <uint64_t> "node": the id of current node
    // <uint64_t> "right": the right child of the node
    // return <int>: 1 if the setting is successful
    int (*set_rightchild)(uint64_t, uint64_t);

    // <uint64_t> "node": the id of current node
    // return <rb_color_t>: the color of the node
    rb_color_t (*get_color)(uint64_t);
    // <uint64_t> "node": the id of current node
    // <rb_color_t> "color": the red-black color of the node
    // return <int>: 1 if the setting is successful
    int (*set_color)(uint64_t, rb_color_t);

    // <uint64_t> "node": the id of current node
    // return <uint64_t>: the key of the node
    uint64_t (*get_key)(uint64_t);
    // <uint64_t> "node": the id of current node
    // <uint64_t> "key": the key of the node
    // return <int>: 1 if the setting is successful
    int (*set_key)(uint64_t, uint64_t);

    // <uint64_t> "node": the id of current node
    // return <uint64_t>: the value of the node
    uint64_t (*get_value)(uint64_t);
    // <uint64_t> "node": the id of current node
    // <uint64_t> "key": the value of the node
    // return <int>: 1 if the setting is successful
    int (*set_value)(uint64_t, uint64_t);
} rbtree_node_interface;

#define IRBT_CONSTRUCT (1 << 0)
#define IRBT_DESTRUCT (1 << 1)
#define IRBT_CHECKNULL (1 << 2)
#define IRBT_COMPARE (1 << 3)
#define IRBT_PARENT (1 << 4)
#define IRBT_LEFT (1 << 5)
#define IRBT_RIGHT (1 << 6)
#define IRBT_COLOR (1 << 7)
#define IRBT_KEY (1 << 8)
#define IRBT_VALUE (1 << 9)
void rbt_validate_interface(rbtree_node_interface *i_node,
    uint64_t flags);

// internal class of the red-black tree
typedef struct RBTREE_INTERNAL_STRUCT
{
    // the root node of the tree
    uint64_t root;

    // this: this pointer
    // <uint64_t> "node": the id of new root node
    // return <int>: 1 if the updating is successful
    int (*update_root)(struct RBTREE_INTERNAL_STRUCT *this, uint64_t);
} rbtree_internal_t;

typedef enum
{
    NO_CHILD = -1,
    LEFT_CHILD = 0,
    RIGHT_CHILD = 1,
} child_t;

// The red-black tree implementation open to other data structures
// especially useful for malloc explicit list implementation
// and vm_area_struct
void rbt_internal_insert(rbtree_internal_t *tree,
    rbtree_node_interface *i_node, 
    uint64_t node_id);
void rbt_internal_delete(rbtree_internal_t *tree,
    rbtree_node_interface *i_node, 
    uint64_t node_id);

//
//  The default implementation of red-black tree
//

typedef struct RB_NODE_STRUCT
{
    // pointers
    struct RB_NODE_STRUCT *parent;
    struct RB_NODE_STRUCT *left; 
    struct RB_NODE_STRUCT *right;

    // edge color to parent
    rb_color_t color;

    // tree node key
    uint64_t key;

    // tree node values
    uint64_t value;
} rb_node_t;

typedef union
{
    rbtree_internal_t base;
    // this struct is the same with rbtree_internal_t
    struct
    {
        // the root node of the tree
        uint64_t root;

        // this: this pointer
        // <uint64_t> "node": the id of new root node
        // return <int>: 1 if the updating is successful
        int (*update_root)(struct RBTREE_INTERNAL_STRUCT *this, uint64_t);
    };
} rb_tree_t;

rb_tree_t *rbt_construct();
void rbt_free(rb_tree_t *tree);
void rbt_add(rb_tree_t *tree, uint64_t key);
void rbt_insert(rb_tree_t *tree, rb_node_t *node);
void rbt_remove(rb_tree_t *tree, uint64_t key);
void rbt_delete(rb_tree_t *tree, rb_node_t *node);
rb_node_t *rbt_find(rb_tree_t *tree, uint64_t key);
rb_node_t *rbt_find_succ(rb_tree_t *tree, uint64_t key);

/*======================================*/
/*      Binary Search Tree              */
/*======================================*/
void bst_internal_insert(rbtree_internal_t *tree,
    rbtree_node_interface *i_node, 
    uint64_t node_id);
void bst_internal_delete(rbtree_internal_t *tree,
    rbtree_node_interface *i_node, 
    uint64_t node_id, int is_rbt,
    uint64_t *db_parent);
uint64_t bst_internal_find(rbtree_internal_t *tree, 
    rbtree_node_interface *i_node, 
    uint64_t key);
uint64_t bst_internal_find_succ(rbtree_internal_t *tree, 
    rbtree_node_interface *i_node, 
    uint64_t key);

rb_tree_t *bst_construct();
void bst_free(rb_tree_t *tree);
void bst_add(rb_tree_t *tree, uint64_t key);
void bst_insert(rb_tree_t *tree, rb_node_t *node);
void bst_remove(rb_tree_t *tree, uint64_t key);
void bst_delete(rb_tree_t *tree, rb_node_t *node);
rb_node_t *bst_find(rb_tree_t *tree, uint64_t key);
rb_node_t *bst_find_succ(rb_tree_t *tree, uint64_t key);

#endif
