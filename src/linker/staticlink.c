/* BCST - Introduction to Computer Systems
 * Author:      yangminz@outlook.com
 * Github:      https://github.com/yangminz/bcst_csapp
 * Bilibili:    https://space.bilibili.com/4564101
 * Zhihu:       https://www.zhihu.com/people/zhao-yang-min
 * This project (code repository and videos) is exclusively owned by yangminz 
 * and shall not be used for commercial and profitting purpose 
 * without yangminz's permission.
 */

#include<stdlib.h>
#include<stdio.h>
#include<assert.h>
#include<string.h>
#include<headers/linker.h>
#include<headers/common.h>

#define MAX_SYMBOL_MAP_LENGTH 64
#define MAX_SECTION_BUFFER_LENGTH 64

// internal mapping between source and destination symbol entries
typedef struct
{
    elf_t       *elf;   // src elf file
    st_entry_t  *src;   // src symbol
    // TODO:
    // relocation entry (referencing section, referenced symbol) converted to (referencing symbol, referenced symbol) entry
    st_entry_t  *dst;   // dst symbol: used for relocation - find the function referencing the undefined symbol
} smap_t;

static void symbol_processing(elf_t **srcs, int num_srcs, elf_t *dst,
    smap_t *map, int *smap_count);
static void simple_resolution(st_entry_t *src, smap_t *map);

/* ------------------------------------ */
/* Exposed Interface for Static Linking */
/* ------------------------------------ */
void link_elf(elf_t **srcs, int num_srcs, elf_t *dst)
{
    // reset the destination since it's a new file
    memset(dst, 0, sizeof(elf_t));

    // create the map table to connect the source elf files and destination elf file symbols
    int smap_count = 0;
    smap_t smap_table[MAX_SYMBOL_MAP_LENGTH];

    // update the smap table - symbol processing
    symbol_processing(srcs, num_srcs, dst,
        (smap_t *)&smap_table, &smap_count);

    // TODO: compute dst Section Header Table and write into buffer
    
    // TODO: merge the symbol content from ELF src into dst sections
}

static void symbol_processing(elf_t **srcs, int num_srcs, elf_t *dst,
    smap_t *map, int *smap_count)
{
    // for every elf files
    for (int i = 0; i < num_srcs; ++ i)
    {
        elf_t *elfp = srcs[i];

        // for every symbol from this elf file
        for (int j = 0; j < elfp->symt_count; ++ j)
        {
            st_entry_t *sym = &(elfp->symt[j]);

            if (sym->bind == STB_LOCAL)
            {
                // insert the static (local) symbol to new elf with confidence:
                // compiler would check if the symbol is redeclared in one *.c file
                assert(*smap_count < MAX_SYMBOL_MAP_LENGTH);
                // TODO: source symbol should be used by destination symbol
            }
            else
            {
                // for other bind: STB_GLOBAL, STB_WEAK, etc. it's possible to have name conflict
                // check if this symbol has been cached in the map
                for (int k = 0; k < *smap_count; ++ k)
                {
                    // TODO: check name conflict
                    // what if the cached symbol is STB_LOCAL?
                    if (1)
                    {
                        // having name conflict, do simple symbol resolution
                        // pick one symbol from current sym and cached map[k]
                        simple_resolution(sym, &map[k]);
                        goto NEXT_SYMBOL_PROCESS;
                    }
                }
                // cache current symbol sym to the map since there is no name conflict
                assert(*smap_count <= MAX_SYMBOL_MAP_LENGTH);
                // TODO: update map table
            }
            NEXT_SYMBOL_PROCESS:
            // do nothing
            ;
        }
    }
    
    // all the elf files have been processed
    // cleanup: check if there is any undefined symbols in the map table
    for (int i = 0; i < *smap_count; ++ i)
    {
        // TODO: check SHN_UNDEF here
        // TODO: the remaining COMMON go to .bss
    }
}

static inline int symbol_precedence(st_entry_t *sym)
{
    // use inline function to imporve efficiency in run-time by preprocessing
    // TODO: get precedence of each symbol
    if (strcmp(sym->st_shndx, "SHN_UNDEF") == 0)
    {
        // Undefined: symbols referenced but not assigned a storage address
        return 0;
    }
    debug_printf(DEBUG_LINKER, "symbol resolution: cannot determine the symbol \"%s\" precedence", sym->st_name);
    exit(0);
}

static void simple_resolution(st_entry_t *src, smap_t *map)
{
    // src: symbol from current elf file
    // dst: pointer to the internal map table slot: src -> dst
    
    // determines which symbol is the one to be kept with 3 rules
    // rule 1: multiple strong symbols with the same name are not allowed
    // rule 2: given a strong symbol and multiple weak symbols with the same name, choose the strong symbol
    // rule 3: given multiple weak symbols with the same name, choose any of the weak symbols
    int src_pre = symbol_precedence(src);
    int dst_pre = symbol_precedence(map->src);

    // TODO: implement rule 1
    if (src_pre == dst_pre)
    {
        debug_printf(DEBUG_LINKER, 
            "symbol resolution: strong symbol \"%s\" is redeclared\n", src->st_name);
        exit(0);
    }

    // TODO: implement rule 3
    if (src_pre == dst_pre)
    {
        return;
    }

    // rule 2 and rule 3: old src is overriden by the current src: src has higher precedence
    map->src = src;
}