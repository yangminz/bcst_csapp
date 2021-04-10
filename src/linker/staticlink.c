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
#define MAX_RELOCATION_LINES 64

// internal mapping between source and destination symbol entries
typedef struct
{
    elf_t       *elf;   // src elf file
    st_entry_t  *src;   // src symbol
    st_entry_t  *dst;   // dst symbol: used for relocation - find the function referencing the undefined symbol
} smap_t;

/* ------------------------------------ */
/* Symbol Processing                    */
/* ------------------------------------ */
static void symbol_processing(elf_t **srcs, int num_srcs, elf_t *dst,
    smap_t *smap_table, int *smap_count);
static void simple_resolution(st_entry_t *sym, elf_t *sym_elf, smap_t *candidate);

/* ------------------------------------ */
/* Section Merging                      */
/* ------------------------------------ */
static void compute_section_header(elf_t *dst, smap_t *smap_table, int *smap_count);
static void merge_section(elf_t **srcs, int num_srcs, elf_t *dst,
    smap_t *smap_table, int *smap_count);

/* ------------------------------------ */
/* Relocation                           */
/* ------------------------------------ */
static void relocation_processing(elf_t **srcs, int num_srcs, elf_t *dst,
    smap_t *smap_table, int *smap_count);

static void R_X86_64_32_handler(elf_t *dst, sh_entry_t *sh,
    int row_referencing, int col_referencing, int addend,
    st_entry_t *sym_referenced);
static void R_X86_64_PC32_handler(elf_t *dst, sh_entry_t *sh,
    int row_referencing, int col_referencing, int addend,
    st_entry_t *sym_referenced);
static void R_X86_64_PLT32_handler(elf_t *dst, sh_entry_t *sh,
    int row_referencing, int col_referencing, int addend,
    st_entry_t *sym_referenced);

typedef void (*rela_handler_t)(elf_t *dst, sh_entry_t *sh,
    int row_referencing, int col_referencing, int addend,
    st_entry_t *sym_referenced);

static rela_handler_t handler_table[3] = {
    &R_X86_64_32_handler,       // 0
    &R_X86_64_PC32_handler,     // 1
    // linux commit b21ebf2: x86: Treat R_X86_64_PLT32 as R_X86_64_PC32
    &R_X86_64_PC32_handler,     // 2
};

/* ------------------------------------ */
/* Helper                               */
/* ------------------------------------ */
static const char *get_stb_string(st_bind_t bind);
static const char *get_stt_string(st_type_t type);

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

    // compute dst Section Header Table and write into buffer
    // UPDATE section headert table: compute runtime address of each section
    // UPDATE buffer: EOF file header: file line count, section header table line count, section header table
    // compute running address of each section: .text, .rodata, .data, .symtab
    // eof starting from 0x00400000
    compute_section_header(dst, smap_table, &smap_count);

    // malloc the dst.symt
    dst->symt_count = smap_count;
    dst->symt = malloc(dst->symt_count * sizeof(st_entry_t));

    // to this point, the EOF file header and section header table is placed
    // merge the left sections and relocate the entries in .text and .data
    
    // merge the symbol content from ELF src into dst sections
    merge_section(srcs, num_srcs, dst, smap_table, &smap_count);

    printf("----------------------\n");
    printf("after merging the sections\n");
    for (int i = 0; i < dst->line_count; ++ i)
    {
        printf("%s\n", dst->buffer[i]);
    }

    // relocating: update the relocation entries from ELF files into EOF buffer
    relocation_processing(srcs, num_srcs, dst, smap_table, &smap_count);

    // finally, check EOF file
    if ((DEBUG_VERBOSE_SET & DEBUG_LINKER) != 0)
    {
        printf("----\nfinal output EOF:\n");
        for (int i = 0; i < dst->line_count; ++ i)
        {
            printf("%s\n", dst->buffer[i]);
        }
    }
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

    if (pre1 == 2 && pre2 == 2)
    {
        /* rule 1
                pre1    pre2
            ---------------------
                2       2
         */
        debug_printf(DEBUG_LINKER, 
            "symbol resolution: strong symbol \"%s\" is redeclared\n", sym->st_name);
        exit(0);
    }
    else if (pre1 != 2 && pre2 != 2)
    {
        /* rule 3 - select higher precedence
                pre1    pre2
            ---------------------
                0       0
                0       1
                1       0
                1       1
         */
        // use the stronger one as best match
        if (pre1 > pre2)
        {
            // select sym as best match
            candidate->src = sym;
            candidate->elf = sym_elf;
        }
        return;
    }
    else if (pre1 == 2)
    {
        /* rule 2 - select current symbol
                pre1    pre2
            ---------------------
                2       0
                2       1
         */
        // select sym as best match
        candidate->src = sym;
        candidate->elf = sym_elf;
    }
    /* rule 2 - select candidate
            pre1    pre2
        ---------------------
            0       2
            1       2
    */
}

static void compute_section_header(elf_t *dst, smap_t *smap_table, int *smap_count)
{
    // we only have .text, .rodata, .data as symbols in the section
    // .bss is not taking any section memory
    int count_text = 0, count_rodata = 0, count_data = 0;
    for (int i = 0; i < *smap_count; ++ i)
    {
        st_entry_t *sym = smap_table[i].src;

        if (strcmp(sym->st_shndx, ".text") == 0)
        {
            // .text section symbol
            count_text += sym->st_size;
        }
        else if (strcmp(sym->st_shndx, ".rodata") == 0)
        {
            // .rodata section symbol
            count_rodata += sym->st_size;
        }
        else if (strcmp(sym->st_shndx, ".data") == 0)
        {
            // .data section symbol
            count_data += sym->st_size;
        }
    }

    // count the section: with .symtab
    dst->sht_count = (count_text != 0) + (count_rodata != 0) + (count_data != 0) + 1;
    // count the total lines
    dst->line_count = 1 + 1 + dst->sht_count + count_text + count_rodata + count_data + *smap_count;
    // the target dst: line_count, sht_count, sht, .text, .rodata, .data, .symtab
    // print to buffer
    sprintf(dst->buffer[0], "%ld", dst->line_count);
    sprintf(dst->buffer[1], "%ld", dst->sht_count);

    // compute the run-time address of the sections: compact in memory
    uint64_t text_runtime_addr = 0x00400000;
    uint64_t rodata_runtime_addr = text_runtime_addr + count_text * MAX_INSTRUCTION_CHAR * sizeof(char);
    uint64_t data_runtime_addr = rodata_runtime_addr + count_rodata * sizeof(uint64_t);
    uint64_t symtab_runtime_addr = 0; // For EOF, .symtab is not loaded into run-time memory but still on disk

    // write the section header table
    assert(dst->sht == NULL);
    dst->sht = malloc(dst->sht_count * sizeof(sh_entry_t));

    // write in .text, .rodata, .data order
    // the start of the offset
    uint64_t section_offset = 1 + 1 + dst->sht_count;
    int sh_index = 0;
    sh_entry_t *sh = NULL;

    if (count_text > 0)
    {
        // get the pointer
        assert(sh_index < dst->sht_count);
        sh = &(dst->sht[sh_index]);
        
        // write the fields
        strcpy(sh->sh_name, ".text");
        sh->sh_addr = text_runtime_addr;
        sh->sh_offset = section_offset;
        sh->sh_size = count_text;

        // write to buffer
        sprintf(dst->buffer[2 + sh_index], "%s,0x%lx,%ld,%ld",
            sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);

        // update the index
        sh_index ++;
        section_offset += sh->sh_size;
    }
    // else skip .text

    if (count_rodata > 0)
    {
        // get the pointer
        assert(sh_index < dst->sht_count);
        sh = &(dst->sht[sh_index]);
        
        // write the fields
        strcpy(sh->sh_name, ".rodata");
        sh->sh_addr = rodata_runtime_addr;
        sh->sh_offset = section_offset;
        sh->sh_size = count_rodata;

        // write to buffer
        sprintf(dst->buffer[2 + sh_index], "%s,0x%lx,%ld,%ld",
            sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);

        // update the index
        sh_index ++;
        section_offset += sh->sh_size;
    }
    // else skip .rodata
    
    if (count_data > 0)
    {
        // get the pointer
        assert(sh_index < dst->sht_count);
        sh = &(dst->sht[sh_index]);
        
        // write the fields
        strcpy(sh->sh_name, ".data");
        sh->sh_addr = data_runtime_addr;
        sh->sh_offset = section_offset;
        sh->sh_size = count_data;

        // write to buffer
        sprintf(dst->buffer[2 + sh_index], "%s,0x%lx,%ld,%ld",
            sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);

        // update the index
        sh_index ++;
        section_offset += sh->sh_size;
    }
    // else skip .data

    // .symtab
    // get the pointer
    assert(sh_index < dst->sht_count);
    sh = &(dst->sht[sh_index]);
    
    // write the fields
    strcpy(sh->sh_name, ".symtab");
    sh->sh_addr = symtab_runtime_addr;
    sh->sh_offset = section_offset;
    sh->sh_size = *smap_count;

    // write to buffer
    sprintf(dst->buffer[2 + sh_index], "%s,0x%lx,%ld,%ld",
        sh->sh_name, sh->sh_addr, sh->sh_offset, sh->sh_size);

    assert(sh_index + 1 == dst->sht_count);

    // print and check
    if ((DEBUG_VERBOSE_SET & DEBUG_LINKER) != 0)
    {
        printf("----------------------\n");
        printf("Destination ELF's SHT in Buffer:\n");
        for (int i = 0; i < 2 + dst->sht_count; ++ i)
        {
            printf("%s\n", dst->buffer[i]);
        }
    }
}

// precondition: dst should know the section offset of each section
// merge the target section lines from ELF files and update dst symtab
static void merge_section(elf_t **srcs, int num_srcs, elf_t *dst,
    smap_t *smap_table, int *smap_count)
{
    int line_written = 1 + 1 + dst->sht_count;
    int symt_written = 0;
    int sym_section_offset = 0;

    for (int section_index = 0; section_index < dst->sht_count; ++ section_index)
    {
        // merge by the dst.sht order in symbol unit

        // get the section by section id
        sh_entry_t *target_sh = &dst->sht[section_index];
        sym_section_offset = 0;
        debug_printf(DEBUG_LINKER, "merging section '%s'\n", target_sh->sh_name);

        // merge the sections
        // scan every input ELF file
        for (int i = 0; i < num_srcs; ++ i)
        {
            debug_printf(DEBUG_LINKER, "\tfrom source elf [%d]\n", i);
            int src_section_index = -1;
            // scan every section in this elf
            for (int j = 0; j < srcs[i]->sht_count; ++ j)
            {
                // check if this ELF srcs[i] contains the same section as target_sh
                if (strcmp(target_sh->sh_name, srcs[i]->sht[j].sh_name) == 0)
                {
                    // we have found the same section name
                    src_section_index = j;
                    // break;
                }
            }

            // check if we have found this target section from src ELF
            if (src_section_index == -1)
            {
                // search for the next ELF
                // because the current ELF srcs[i] does not contain the target_sh
                continue;
            }
            else
            {
                // found the section in this ELF srcs[i]
                // check its symtab
                for (int j = 0; j < srcs[i]->symt_count; ++ j)
                {
                    st_entry_t *sym = &srcs[i]->symt[j];

                    if (strcmp(sym->st_shndx, target_sh->sh_name) == 0)
                    {
                        for (int k = 0; k < *smap_count; ++ k)
                        {
                            // scan the cached dst symbols to check
                            // if this symbol should be merged into this section
                            if (sym == smap_table[k].src)
                            {
                                // exactly the cached symbol
                                debug_printf(DEBUG_LINKER, "\t\tsymbol '%s'\n", sym->st_name);

                                // this symbol should be merged into dst's section target_sh
                                // copy this symbol from srcs[i].buffer into dst.buffer
                                // srcs[i].buffer[sh_offset + st_value, sh_offset + st_value + st_size] inclusive
                                for (int t = 0; t < sym->st_size; ++ t)
                                {
                                    int dst_index = line_written + t;
                                    int src_index = srcs[i]->sht[src_section_index].sh_offset +
                                        sym->st_value + t;
                                    
                                    assert(dst_index < MAX_ELF_FILE_LENGTH);
                                    assert(src_index < MAX_ELF_FILE_LENGTH);
                                    
                                    strcpy(
                                        dst->buffer[dst_index],
                                        srcs[i]->buffer[src_index]);
                                }

                                // copy the symbol table entry from srcs[i].symt[j] to
                                // dst.symt[symt_written]
                                assert(symt_written < dst->symt_count);
                                // copy the entry
                                strcpy(dst->symt[symt_written].st_name, sym->st_name);
                                dst->symt[symt_written].bind = sym->bind;
                                dst->symt[symt_written].type = sym->type;
                                strcpy(dst->symt[symt_written].st_shndx, sym->st_shndx);
                                // MUST NOT BE A COMMON, so the section offset MUST NOT BE alignment
                                dst->symt[symt_written].st_value = sym_section_offset;
                                dst->symt[symt_written].st_size = sym->st_size;

                                // update the smap_table
                                // this will hep the relocation
                                smap_table[k].dst = &dst->symt[symt_written];

                                // udpate the counter
                                symt_written += 1;
                                line_written += sym->st_size;
                                sym_section_offset += sym->st_size;
                            }
                        }
                        // symbol srcs[i].symt[j] has been checked
                    }
                }
                // all symbols in ELF file srcs[i] has been checked
            }
        }
        // dst.sht[section_index] has been merged from src ELFs
    }
    // all .text, .rodata, .data sections in dst has been merged

    // finally, merge .symtab
    for (int i = 0; i < dst->symt_count; ++ i)
    {
        st_entry_t *sym = &dst->symt[i];
        sprintf(dst->buffer[line_written], "%s,%s,%s,%s,%ld,%ld", 
            sym->st_name, get_stb_string(sym->bind), get_stt_string(sym->type),
            sym->st_shndx, sym->st_value, sym->st_size);
        line_written ++;
    }
    assert(line_written == dst->line_count);
}

// precondition: smap_table.dst is valid
static void relocation_processing(elf_t **srcs, int num_srcs, elf_t *dst,
    smap_t *smap_table, int *smap_count)
{
    sh_entry_t *eof_text_sh = NULL;
    sh_entry_t *eof_data_sh = NULL;
    for (int i = 0; i < dst->sht_count; ++ i)
    {
        if (strcmp(dst->sht[i].sh_name, ".text") == 0)
        {
            eof_text_sh = &(dst->sht[i]);
        }
        else if (strcmp(dst->sht[i].sh_name, ".data") == 0)
        {
            eof_data_sh = &(dst->sht[i]);
        }
    }

    // update the relocation entries: r_row, r_col, sym
    for (int i = 0; i < num_srcs; ++ i)
    {
        elf_t *elf = srcs[i];

        // .rel.text
        for (int j = 0; j < elf->reltext_count; ++ j)
        {
            rl_entry_t *r = &elf->reltext[j];

            // search the referencing symbol
            for (int k = 0; k < elf->symt_count; ++ k)
            {
                st_entry_t *sym = &elf->symt[k];

                if (strcmp(sym->st_shndx, ".text") == 0)
                {
                    // must be referenced by a .text symbol
                    // check if this symbol is the one referencing
                    int sym_text_start = sym->st_value;
                    int sym_text_end = sym->st_value + sym->st_size;

                    if (sym_text_start <= r->r_row && r->r_row <= sym_text_end)
                    {
                        // symt[k] is referencing reltext[j].sym
                        // search the smap table to find the EOF location
                        int smap_found = 0;
                        for (int t = 0; t < *smap_count; ++ t)
                        {
                            if (smap_table[t].src == sym)
                            {
                                smap_found = 1;
                                st_entry_t *eof_referencing = smap_table[t].dst;

                                // search the being referenced symbol
                                for (int u = 0; u < *smap_count; ++ u)
                                {
                                    // what is the EOF symbol name?
                                    // how to get the referenced symbol name
                                    if (strcmp(elf->symt[r->sym].st_name, smap_table[u].dst->st_name) == 0 &&
                                        smap_table[u].dst->bind == STB_GLOBAL)
                                    {
                                        // till now, the referencing row and referenced row are all found
                                        // update the location
                                        st_entry_t *eof_referenced = smap_table[u].dst;

                                        (handler_table[(int)r->type])(
                                            dst, eof_text_sh,
                                            r->r_row - sym->st_value + eof_referencing->st_value, 
                                            r->r_col, 
                                            r->r_addend,
                                            eof_referenced);
                                        goto NEXT_REFERENCE_IN_TEXT;
                                    }
                                }
                            }
                        }
                        // referencing must be in smap_table
                        // because it has definition, is a strong symbol
                        assert(smap_found == 1);
                    }
                }
            }
            NEXT_REFERENCE_IN_TEXT:
            ;
        }

        // .rel.data
        for (int j = 0; j < elf->reldata_count; ++ j)
        {
            rl_entry_t *r = &elf->reldata[j];

            // search the referencing symbol
            for (int k = 0; k < elf->symt_count; ++ k)
            {
                st_entry_t *sym = &elf->symt[k];

                if (strcmp(sym->st_shndx, ".data") == 0)
                {
                    // must be referenced by a .data symbol
                    // check if this symbol is the one referencing
                    int sym_data_start = sym->st_value;
                    int sym_data_end = sym->st_value + sym->st_size;

                    if (sym_data_start <= r->r_row && r->r_row <= sym_data_end)
                    {
                        // symt[k] is referencing reldata[j].sym
                        // search the smap table to find the EOF location
                        int smap_found = 0;
                        for (int t = 0; t < *smap_count; ++ t)
                        {
                            if (smap_table[t].src == sym)
                            {
                                smap_found = 1;
                                st_entry_t *eof_referencing = smap_table[t].dst;

                                // search the being referenced symbol
                                for (int u = 0; u < *smap_count; ++ u)
                                {
                                    // what is the EOF symbol name?
                                    // how to get the referenced symbol name
                                    if (strcmp(elf->symt[r->sym].st_name, smap_table[u].dst->st_name) == 0 &&
                                        smap_table[u].dst->bind == STB_GLOBAL)
                                    {
                                        // till now, the referencing row and referenced row are all found
                                        // update the location
                                        st_entry_t *eof_referenced = smap_table[u].dst;

                                        (handler_table[(int)r->type])(
                                            dst, eof_data_sh,
                                            r->r_row - sym->st_value + eof_referencing->st_value, 
                                            r->r_col, 
                                            r->r_addend,
                                            eof_referenced);
                                        goto NEXT_REFERENCE_IN_DATA;
                                    }
                                }
                            }
                        }
                        // referencing must be in smap_table
                        // because it has definition, is a strong symbol
                        assert(smap_found == 1);
                    }
                }
            }
            NEXT_REFERENCE_IN_DATA:
            ;
        }
    }
}

// relocating handlers

static void R_X86_64_32_handler(elf_t *dst, sh_entry_t *sh,
    int row_referencing, int col_referencing, int addend,
    st_entry_t *sym_referenced)
{
    printf("row = %d, col = %d, symbol referenced = %s\n",
        row_referencing, col_referencing, sym_referenced->st_name
    );
}

static void R_X86_64_PC32_handler(elf_t *dst, sh_entry_t *sh,
    int row_referencing, int col_referencing, int addend,
    st_entry_t *sym_referenced)
{
    printf("row = %d, col = %d, symbol referenced = %s\n",
        row_referencing, col_referencing, sym_referenced->st_name
    );
}

static void R_X86_64_PLT32_handler(elf_t *dst, sh_entry_t *sh,
    int row_referencing, int col_referencing, int addend,
    st_entry_t *sym_referenced)
{
    printf("row = %d, col = %d, symbol referenced = %s\n",
        row_referencing, col_referencing, sym_referenced->st_name
    );
}

static const char *get_stb_string(st_bind_t bind)
{
    switch (bind)
    {
        case STB_GLOBAL:
            return "STB_GLOBAL";
        case STB_LOCAL:
            return "STB_LOCAL";
        case STB_WEAK:
            return "STB_WEAK";
        default:
            printf("incorrect symbol bind\n");
            exit(0);
    }
}

static const char *get_stt_string(st_type_t type)
{
    switch (type)
    {
        case STT_NOTYPE:
            return "STT_NOTYPE";
        case STT_OBJECT:
            return "STT_OBJECT";
        case STT_FUNC:
            return "STT_FUNC";
        default:
            printf("incorrect symbol type\n");
            exit(0);
    }
}