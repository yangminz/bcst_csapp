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
#ifndef DEBUG_GUARD
#define DEBUG_GUARD

#include<stdint.h>

#define DEBUG_INSTRUCTIONCYCLE      (0x1)
#define DEBUG_REGISTERS             (0x2)
#define DEBUG_PRINTSTACK            (0x4)
#define DEBUG_PRINTCACHESET         (0x8)
#define DEBUG_CACHEDETAILS          (0x10)
#define DEBUG_MMU                   (0x20)
#define DEBUG_LINKER                (0x40)
#define DEBUG_LOADER                (0x80)
#define DEBUG_PARSEINST             (0x100)

#define DEBUG_VERBOSE_SET           (0x041)

// do page walk
#define DEBUG_ENABLE_PAGE_WALK      (0)

// use sram cache for memory access 
#define DEBUG_ENABLE_SRAM_CACHE     (0)

// printf wrapper
uint64_t debug_printf(uint64_t open_set, const char *format, ...);

// type converter
// uint32 to its equivalent float with rounding
uint32_t uint2float(uint32_t u);

// convert string dec or hex to the integer bitmap
uint64_t string2uint(const char *str);
uint64_t string2uint_range(const char *str, int start, int end);

// commonly shared variables
#define MAX_INSTRUCTION_CHAR (64)

/*======================================*/
/*      clean up events                 */
/*======================================*/
void add_cleanup_event(void *func);
void finally_cleanup();

/*======================================*/
/*      data structures                 */
/*======================================*/

// trie
typedef struct TRIE_NODE_STRUCT
{
    // '0'-'9','a'-'z','%'
    struct TRIE_NODE_STRUCT *next[37];
    uint64_t address;
} trie_node_t;

void trie_insert(trie_node_t **root, char *key, uint64_t val);
int trie_get(trie_node_t *root, char *key, uint64_t *val);
void trie_free(trie_node_t *root);
void trie_print(trie_node_t *root);

// extendible hash table
typedef struct
{
    int localdepth;     // the local depth
    int counter;        // the counter of slots (have data)
    char **karray;
    uint64_t *varray;
} bucket_t;

typedef struct
{
    int size;           // the size of (key, value) tuples of each bucket
    int num;            // number of buckets = 1 << globaldepth
    int globaldepth;    // the global depth

    bucket_t *barray;    // the internal table is actually an array
} hashtable_t;

hashtable_t *hashtable_construct(int bsize);
void hashtable_free(hashtable_t *tab);
int hashtable_get(hashtable_t *tab, char *key, uint64_t *val);
int hashtable_insert(hashtable_t **tab_addr, char *key, uint64_t val);
void print_hashtable(hashtable_t *tab);

#endif
