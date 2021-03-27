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
    st_entry_t  *dst;   // dst symbol: used for relocation - find the function referencing the undefined symbol
    // TODO:
    // relocation entry (referencing section, referenced symbol) converted to (referencing symbol, referenced symbol) entry
} smap_t;

static void symbol_processing(elf_t **srcs, int num_srcs, elf_t *dst,
    smap_t *smap_table, int *smap_count);
static void simple_resolution(st_entry_t *sym, elf_t *sym_elf, smap_t *candidate);

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

    printf("----------------------\n");

    for (int i = 0; i < smap_count; ++ i)
    {
        st_entry_t *ste = smap_table[i].src;
        debug_printf(DEBUG_LINKER, "%s\t%d\t%d\t%s\t%d\t%d\n",
            ste->st_name,
            ste->bind,
            ste->type,
            ste->st_shndx,
            ste->st_value,
            ste->st_size);
    }

    // TODO: compute dst Section Header Table and write into buffer
    
    // TODO: merge the symbol content from ELF src into dst sections
}

static void symbol_processing(elf_t **srcs, int num_srcs, elf_t *dst,
    smap_t *smap_table, int *smap_count)
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
                // even if local symbol has the same name, just insert it into dst
                smap_table[*smap_count].src = sym;
                smap_table[*smap_count].elf = elfp;
                // we have not created dst here
                (*smap_count) ++;
            }
            else if (sym->bind == STB_GLOBAL)
            {
                // for other bind: STB_GLOBAL, etc. it's possible to have name conflict
                // check if this symbol has been cached in the map
                for (int k = 0; k < *smap_count; ++ k)
                {
                    // check name conflict
                    // what if the cached symbol is STB_LOCAL?
                    st_entry_t *candidate = smap_table[k].src;
                    if (candidate->bind == STB_GLOBAL &&
                        (strcmp(candidate->st_name, sym->st_name) == 0))
                    {
                        // having name conflict, do simple symbol resolution
                        // pick one symbol from current sym and cached map[k]
                        simple_resolution(sym, elfp, &smap_table[k]);
                        goto NEXT_SYMBOL_PROCESS;
                    }
                }

                // not find any name conflict
                // cache current symbol sym to the map since there is no name conflict
                assert(*smap_count <= MAX_SYMBOL_MAP_LENGTH);
                // update map table
                smap_table[*smap_count].src = sym;
                smap_table[*smap_count].elf = elfp;
                (*smap_count) ++;
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
        st_entry_t *s = smap_table[i].src;

        // check no more SHN_UNDEF here
        assert(strcmp(s->st_shndx, "SHN_UNDEF") != 0);
        assert(s->type != STT_NOTYPE);

        // the remaining COMMON go to .bss
        if (strcmp(s->st_shndx, "COMMON") == 0)
        {
            char *bss = ".bss";
            for (int j = 0; j < MAX_CHAR_SECTION_NAME; ++ j)
            {
                if (j < 4)
                {
                    s->st_shndx[j] = bss[j];
                }
                else
                {
                    s->st_shndx[j] = '\0';
                }
            }
            s->st_value = 0;
        }
    }
}

static inline int symbol_precedence(st_entry_t *sym)
{
    // use inline function to imporve efficiency in run-time by preprocessing
    /*  we do not consider weak because it's very rare
        and we do not consider local because it's not conflicting

            bind        type        shndx               prec
            --------------------------------------------------
            global      notype      undef               0 - undefined
            global      object      common              1 - tentative
            global      object      data,bss,rodata     2 - defined
            global      func        text                2 - defined
    */
    assert(sym->bind == STB_GLOBAL);

    // get precedence of the symbol
    
    if (strcmp(sym->st_shndx, "SHN_UNDEF") == 0 && sym->type == STT_NOTYPE)
    {
        // Undefined: symbols referenced but not assigned a storage address
        return 0;
    }

    if (strcmp(sym->st_shndx, "COMMON") == 0 && sym->type == STT_OBJECT)
    {
        // Tentative: section to be decided after symbol resolution
        return 1;
    }

    if ((strcmp(sym->st_shndx, ".text") == 0 && sym->type == STT_FUNC) || 
        (strcmp(sym->st_shndx, ".data") == 0 && sym->type == STT_OBJECT) || 
        (strcmp(sym->st_shndx, ".rodata") == 0 && sym->type == STT_OBJECT) || 
        (strcmp(sym->st_shndx, ".bss") == 0 && sym->type == STT_OBJECT))
    {
        // Defined
        return 2;
    }

    debug_printf(DEBUG_LINKER, "symbol resolution: cannot determine the symbol \"%s\" precedence", sym->st_name);
    exit(0);
}

static void simple_resolution(st_entry_t *sym, elf_t *sym_elf, smap_t *candidate)
{
    // sym: symbol from current elf file
    // candidate: pointer to the internal map table slot: src -> dst
    
    // determines which symbol is the one to be kept with 3 rules
    // rule 1: multiple strong symbols with the same name are not allowed
    // rule 2: given a strong symbol and multiple weak symbols with the same name, choose the strong symbol
    // rule 3: given multiple weak symbols with the same name, choose any of the weak symbols
    int pre1 = symbol_precedence(sym);
    int pre2 = symbol_precedence(candidate->src);

    // implement rule 1
    if (pre1 == 2 && pre2 == 2)
    {
        debug_printf(DEBUG_LINKER, 
            "symbol resolution: strong symbol \"%s\" is redeclared\n", sym->st_name);
        exit(0);
    }

    // implement rule 3
    if (pre1 != 2 && pre2 != 2)
    {
        // use the stronger one as best match
        if (pre1 > pre2)
        {
            // select sym as best match
            candidate->src = sym;
            candidate->elf = sym_elf;
        }
        return;
    }

    // implement rule 2
    if (pre1 == 2)
    {
        // select sym as best match
        candidate->src = sym;
        candidate->elf = sym_elf;
    }
}