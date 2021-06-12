/* BCST - Introduction to Computer Systems
 * Author:      yangminz@outlook.com
 * Github:      https://github.com/yangminz/bcst_csapp
 * Bilibili:    https://space.bilibili.com/4564101
 * Zhihu:       https://www.zhihu.com/people/zhao-yang-min
 * This project (code repository and videos) is exclusively owned by yangminz 
 * and shall not be used for commercial and profitting purpose 
 * without yangminz's permission.
 */

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "headers/linker.h"
#include "headers/common.h"

#ifdef DEBUG_LINK

static void print_sh_entry(sh_entry_t *sh)
{
    printf("%s\t%lx\t%lu\t%lu\n",
        sh->sh_name,
        sh->sh_addr,
        sh->sh_offset,
        sh->sh_size);
}

static void print_symtab_entry(st_entry_t *ste)
{
    printf("%s\t%d\t%d\t%s\t%lu\t%lu\n",
        ste->st_name,
        ste->bind,
        ste->type,
        ste->st_shndx,
        ste->st_value,
        ste->st_size);
}

static void print_relocation_entry(rl_entry_t *rte)
{
    printf("%lu\t%lu\t%d\t%u\t%ld\n",
        rte->r_row,
        rte->r_col,
        rte->type,
        rte->sym,
        rte->r_addend);
}

#endif

static int parse_table_entry(char *str, char ***ent)
{
    // column0,column1,column2,column3,...
    // parse line as table entries
    int count_col = 1;
    int len = strlen(str);

    // count columns
    for (int i = 0; i < len; ++ i)
    {
        if (str[i] == ',')
        {
            count_col ++;
        }
    }

    // malloc and create list
    char **arr = malloc(count_col * sizeof(char *));
    *ent = arr;

    int col_index = 0;
    int col_width = 0;
    char col_buf[32];
    for (int i = 0; i < len + 1; ++ i)
    {
        if (str[i] == ',' || str[i] == '\0')
        {
            assert(col_index < count_col);

            // malloc and copy
            char *col = malloc((col_width + 1) * sizeof(char));
            for (int j = 0; j < col_width; ++ j)
            {
                col[j] = col_buf[j];
            }
            col[col_width] = '\0';

            // update
            arr[col_index] = col;
            col_index ++;
            col_width = 0;
        }
        else
        {
            assert(col_width < 32);
            col_buf[col_width] = str[i];
            col_width ++;
        }
    }
    return count_col;
}

static void parse_sh(char *str, sh_entry_t *sh)
{
    // .text,0x0,4,22
    char **cols;
    int num_cols = parse_table_entry(str, &cols);
    assert(num_cols == 4);

    assert(sh != NULL);
    strcpy(sh->sh_name, cols[0]);
    sh->sh_addr = string2uint(cols[1]);
    sh->sh_offset = string2uint(cols[2]);
    sh->sh_size = string2uint(cols[3]);

    for (int i = 0; i < num_cols; ++ i)
    {
        free(cols[i]);
    }
    free(cols);
}

static void parse_symtab(char *str, st_entry_t *ste)
{
    // sum,STB_GLOBAL,STT_FUNCTION,.text,0,22
    char **cols;
    int num_cols = parse_table_entry(str, &cols);
    assert(num_cols == 6);

    assert(ste != NULL);
    strcpy(ste->st_name, cols[0]);

    // select symbol bind
    uint64_t bind_value;
    if (hashtable_get(link_constant_dict, cols[1], &bind_value) == 0)
    {
        // failed
        printf("symbol bind is neiter LOCAL, GLOBAL, nor WEAK\n");
        exit(0);
    }
    ste->bind = (st_bind_t)bind_value;

    uint64_t type_value;
    if (hashtable_get(link_constant_dict, cols[2], &type_value) == 0)
    {
        // failed
        printf("symbol type is neiter NOTYPE, OBJECT, nor FUNC\n");
        exit(0);
    }
    ste->type = (st_type_t)type_value;

    strcpy(ste->st_shndx, cols[3]);

    ste->st_value = string2uint(cols[4]);
    ste->st_size = string2uint(cols[5]);

    for (int i = 0; i < num_cols; ++ i)
    {
        free(cols[i]);
    }
    free(cols);
}

static void parse_relocation(char *str, rl_entry_t *rte)
{
    // 4,7,R_X86_64_PC32,0,-4
    char **cols;
    int num_cols = parse_table_entry(str, &cols);
    assert(num_cols == 5);

    assert(rte != NULL);
    rte->r_row = string2uint(cols[0]);
    rte->r_col = string2uint(cols[1]);

    // select relocation type
    uint64_t type_value;
    if (hashtable_get(link_constant_dict, cols[2], &type_value) == 0)
    {
        // failed
        printf("relocation type is neiter R_X86_64_32, R_X86_64_PC32, nor R_X86_64_PLT32\n");
        exit(0);
    }
    rte->type = (st_type_t)type_value;

    rte->sym = string2uint(cols[3]);

    uint64_t bitmap = string2uint(cols[4]);
    rte->r_addend = *(int64_t *)&bitmap;

    for (int i = 0; i < num_cols; ++ i)
    {
        free(cols[i]);
    }
    free(cols);
}

static int read_elf(const char *filename, uint64_t bufaddr)
{
    // open file and read
    FILE *fp;
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
#ifdef DEBUG_LINK
        printf("unable to open file %s\n", filename);
#endif
        exit(1);
    }

    // read text file line by line
    char line[MAX_ELF_FILE_WIDTH];
    int line_counter = 0;

    while (fgets(line, MAX_ELF_FILE_WIDTH, fp) != NULL)
    {
        int len = strlen(line);
        if ((len == 0) || 
            (len >= 1 && (line[0] == '\n' || line[0] == '\r')) ||
            (len >= 2 && (line[0] == '/' && line[1] == '/')))
        {
            continue;
        }

        // check if is empty or white line
        int iswhite = 1;
        for (int i = 0; i < len; ++ i)
        {
            iswhite = iswhite && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r');
        }
        if (iswhite == 1)
        {
            continue;
        }

        // to this line, this line is not white and contains information

        if (line_counter < MAX_ELF_FILE_LENGTH)
        {
            // store this line to buffer[line_counter]
            uint64_t addr = bufaddr + line_counter * MAX_ELF_FILE_WIDTH * sizeof(char);
            char *linebuf = (char *)addr;

            int i = 0;
            while (i < len && i < MAX_ELF_FILE_WIDTH)
            {
                if ((line[i] == '\n') || 
                    (line[i] == '\r') || 
                    ((i + 1 < len) && (i + 1 < MAX_ELF_FILE_WIDTH) && line[i] == '/' && line[i + 1] == '/'))
                {
                    break;
                }
                linebuf[i] = line[i];
                i ++;
            }
            linebuf[i] = '\0';
            line_counter ++;
        }
        else
        {
#ifdef DEBUG_LINK
            printf("elf file %s is too long (>%d)\n", filename, MAX_ELF_FILE_LENGTH);
#endif
            fclose(fp);
            exit(1);
        }
    }

    fclose(fp);
    assert(string2uint((char *)bufaddr) == line_counter);
    return line_counter;
}

static void free_link_constant_dict()
{
    hashtable_free(link_constant_dict);
}

static void init_dictionary()
{
    if (link_constant_dict != NULL)
    {
        return;
    }

    link_constant_dict = hashtable_construct(4);

    link_constant_dict = hashtable_insert(link_constant_dict, "STB_LOCAL", STB_LOCAL);
    link_constant_dict = hashtable_insert(link_constant_dict, "STB_GLOBAL", STB_GLOBAL);
    link_constant_dict = hashtable_insert(link_constant_dict, "STB_WEAK", STB_WEAK);

    link_constant_dict = hashtable_insert(link_constant_dict, "STT_NOTYPE", STT_NOTYPE);
    link_constant_dict = hashtable_insert(link_constant_dict, "STT_OBJECT", STT_OBJECT);
    link_constant_dict = hashtable_insert(link_constant_dict, "STT_FUNC", STT_FUNC);

    link_constant_dict = hashtable_insert(link_constant_dict, "R_X86_64_32", R_X86_64_32);
    link_constant_dict = hashtable_insert(link_constant_dict, "R_X86_64_PC32", R_X86_64_PC32);
    link_constant_dict = hashtable_insert(link_constant_dict, "R_X86_64_PLT32", R_X86_64_PLT32);

    add_cleanup_event(free_link_constant_dict);
}

void parse_elf(char *filename, elf_t *elf)
{
    assert(elf != NULL);
    elf->line_count = read_elf(filename, (uint64_t)(&(elf->buffer)));
    for (int i = 0; i < elf->line_count; ++ i)
    {
        printf("[%d]\t%s\n", i, elf->buffer[i]);
    }

    init_dictionary();

    // parse section headers
    elf->sht_count = string2uint(elf->buffer[1]);
    elf->sht = malloc(elf->sht_count * sizeof(sh_entry_t));
    memset(elf->sht, 0, elf->sht_count * sizeof(sh_entry_t));

    sh_entry_t *symt_sh = NULL;
    sh_entry_t *rtext_sh = NULL;
    sh_entry_t *rdata_sh = NULL;
    for (int i = 0; i < elf->sht_count; ++ i)
    {
        parse_sh(elf->buffer[2 + i], &(elf->sht[i]));
#ifdef DEBUG_LINK
        print_sh_entry(&(elf->sht[i]));
#endif

        if (strcmp(elf->sht[i].sh_name, ".symtab") == 0)
        {
            // this is the section header for symbol table
            symt_sh = &(elf->sht[i]);
        }
        else if (strcmp(elf->sht[i].sh_name, ".rel.text") == 0)
        {
            // this is the section header for .rel.text
            rtext_sh = &(elf->sht[i]);
        }
        else if (strcmp(elf->sht[i].sh_name, ".rel.data") == 0)
        {
            // this is the section header for .rel.dat
            rdata_sh = &(elf->sht[i]);
        }
    }

    assert(symt_sh != NULL);

    // parse symbol table
    elf->symt_count = symt_sh->sh_size;
    elf->symt = malloc(elf->symt_count * sizeof(st_entry_t));
    memset(elf->symt, 0, elf->symt_count * sizeof(st_entry_t));

    for (int i = 0; i < symt_sh->sh_size; ++ i)
    {
        parse_symtab(
            elf->buffer[i + symt_sh->sh_offset],
            &(elf->symt[i]));
#ifdef DEBUG_LINK
        print_symtab_entry(&(elf->symt[i]));
#endif
    }

    // parse relocation table
    if (rtext_sh != NULL)
    {
        elf->reltext_count = rtext_sh->sh_size;
        elf->reltext = malloc(elf->reltext_count * sizeof(rl_entry_t));
        memset(elf->reltext, 0, elf->reltext_count * sizeof(rl_entry_t));

        for (int i = 0; i < rtext_sh->sh_size; ++ i)
        {
            parse_relocation(
                elf->buffer[i + rtext_sh->sh_offset],
                &(elf->reltext[i])
            );
            int st = elf->reltext[i].sym;
            assert(0 <= st && st < elf->symt_count);

#ifdef DEBUG_LINK
            print_relocation_entry(&(elf->reltext[i]));
#endif
        }
    }
    else
    {
        elf->reltext_count = 0;
        elf->reltext = NULL;
    }

    // .rel.data
    if (rdata_sh != NULL)
    {
        elf->reldata_count = rdata_sh->sh_size;
        elf->reldata = malloc(elf->reldata_count * sizeof(rl_entry_t));
        memset(elf->reldata, 0, elf->reldata_count * sizeof(rl_entry_t));

        for (int i = 0; i < rdata_sh->sh_size; ++ i)
        {
            parse_relocation(
                elf->buffer[i + rdata_sh->sh_offset],
                &(elf->reldata[i])
            );
            int st = elf->reldata[i].sym;
            assert(0 <= st && st < elf->symt_count);

#ifdef DEBUG_LINK
            print_relocation_entry(&(elf->reldata[i]));
#endif
        }
    }
    else
    {
        elf->reldata_count = 0;
        elf->reldata = NULL;
    }
}

void write_eof(const char *filename, elf_t *eof)
{
    // open elf file
    FILE *fp;
    fp = fopen(filename, "w");
    if (fp == NULL)
    {
#ifdef DEBUG_LINK
        printf("unable to open file %s\n", filename);
#endif 
        exit(1);
    }

    for (int i = 0; i < eof->line_count; ++ i)
    {
        fprintf(fp, "%s\n", eof->buffer[i]);
    }

    fclose(fp);

    // free hash table
    hashtable_free(link_constant_dict);
}

void free_elf(elf_t *elf)
{
    assert(elf != NULL);

    if (elf->sht != NULL)
    {
        free(elf->sht);
    }

    if (elf->symt != NULL)
    {
        free(elf->symt);
    }

    if (elf->reltext != NULL)
    {
        free(elf->reltext);
    }
    
    if (elf->reldata != NULL)
    {
        free(elf->reldata);
    }
    
    free(elf);
}

#ifdef DEBUG_PARSE_ELF
int main()
{
    printf("Testing parsing ELF file ...\n");

    elf_t *elf_file= malloc(sizeof(elf_t));
    parse_elf("./files/exe/main.elf.txt", elf_file);

    assert(elf_file->line_count == 25);
    assert(elf_file->sht_count == 4);
    assert(elf_file->symt_count == 4);
    assert(elf_file->reltext_count == 2);
    assert(elf_file->reldata_count == 0);

    assert(strcmp(elf_file->sht[0].sh_name, ".text") == 0);
    assert(elf_file->sht[0].sh_addr == 0);
    assert(elf_file->sht[0].sh_offset == 6);
    assert(elf_file->sht[0].sh_size == 10);

    assert(strcmp(elf_file->sht[1].sh_name, ".data") == 0);
    assert(elf_file->sht[1].sh_addr == 0);
    assert(elf_file->sht[1].sh_offset == 16);
    assert(elf_file->sht[1].sh_size == 3);

    assert(strcmp(elf_file->sht[2].sh_name, ".symtab") == 0);
    assert(elf_file->sht[2].sh_addr == 0);
    assert(elf_file->sht[2].sh_offset == 19);
    assert(elf_file->sht[2].sh_size == 4);

    assert(strcmp(elf_file->sht[3].sh_name, ".rel.text") == 0);
    assert(elf_file->sht[3].sh_addr == 0);
    assert(elf_file->sht[3].sh_offset == 23);
    assert(elf_file->sht[3].sh_size == 2);
    assert(strcmp(elf_file->symt[0].st_name, "array") == 0);
    assert(elf_file->symt[0].bind == STB_GLOBAL);
    assert(elf_file->symt[0].type == STT_OBJECT);
    assert(strcmp(elf_file->symt[0].st_shndx, ".data") == 0);
    assert(elf_file->symt[0].st_value == 0);
    assert(elf_file->symt[0].st_size == 2);

    assert(strcmp(elf_file->symt[1].st_name, "bias") == 0);
    assert(elf_file->symt[1].bind == STB_GLOBAL);
    assert(elf_file->symt[1].type == STT_OBJECT);
    assert(strcmp(elf_file->symt[1].st_shndx, ".data") == 0);
    assert(elf_file->symt[1].st_value == 2);
    assert(elf_file->symt[1].st_size == 1);

    assert(strcmp(elf_file->symt[2].st_name, "main") == 0);
    assert(elf_file->symt[2].bind == STB_GLOBAL);
    assert(elf_file->symt[2].type == STT_FUNC);
    assert(strcmp(elf_file->symt[2].st_shndx, ".text") == 0);
    assert(elf_file->symt[2].st_value == 0);
    assert(elf_file->symt[2].st_size == 10);

    assert(strcmp(elf_file->symt[3].st_name, "sum") == 0);
    assert(elf_file->symt[3].bind == STB_GLOBAL);
    assert(elf_file->symt[3].type == STT_NOTYPE);
    assert(strcmp(elf_file->symt[3].st_shndx, "SHN_UNDEF") == 0);
    assert(elf_file->symt[3].st_value == 0);
    assert(elf_file->symt[3].st_size == 0);

    assert(elf_file->reltext[0].r_row == 4);
    assert(elf_file->reltext[0].r_col == 7);
    assert(elf_file->reltext[0].type == R_X86_64_PC32);
    assert(elf_file->reltext[0].sym == 0);
    assert(elf_file->reltext[0].r_addend == -4);

    assert(elf_file->reltext[1].r_row == 5);
    assert(elf_file->reltext[1].r_col == 7);
    assert(elf_file->reltext[1].type == R_X86_64_PLT32);
    assert(elf_file->reltext[1].sym == 3);
    assert(elf_file->reltext[1].r_addend == -4);

    free_elf(elf_file);

    finally_cleanup();

    printf("\tPass\n.");
}
#endif