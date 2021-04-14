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
#ifndef LINKER_GUARD
#define LINKER_GUARD

#include <stdint.h>
#include <stdlib.h>
#include "headers/datastruct.h"

#define MAX_CHAR_SECTION_NAME (32)

typedef struct
{
    char sh_name[MAX_CHAR_SECTION_NAME];
    uint64_t sh_addr;
    uint64_t sh_offset;     // line offset or effective line index
    uint64_t sh_size;
} sh_entry_t;

#define MAX_CHAR_SYMBOL_NAME (64)

typedef enum
{
    STB_LOCAL,
    STB_GLOBAL,
    STB_WEAK
} st_bind_t;

typedef enum
{
    STT_NOTYPE,
    STT_OBJECT,
    STT_FUNC
} st_type_t;

typedef struct
{
    char st_name[MAX_CHAR_SECTION_NAME];
    st_bind_t bind;
    st_type_t type;
    char st_shndx[MAX_CHAR_SECTION_NAME];
    uint64_t st_value;      // in-section offset
    uint64_t st_size;       // count of lines of symbol
} st_entry_t;

/*======================================*/
/*      relocation information          */
/*======================================*/

typedef enum
{
    R_X86_64_32,
    R_X86_64_PC32,
    R_X86_64_PLT32,
} reltype_t;

hashtable_t *link_constant_dict;

// relocation entry type
typedef struct
{
    /*  this is what's different in our implementation. instead of byte offset, 
        we use line index + char offset to locate the symbol */
    uint64_t    r_row;      // line index of the symbol in buffer section
                            // for .rel.text, that's the line index in .text section
                            // for .rel.data, that's the line index in .data section
    uint64_t    r_col;      // char offset in the buffer line
    reltype_t   type;       // relocation type
    uint32_t    sym;        // symbol table index
    int64_t     r_addend;   // constant part of relocation expression
} rl_entry_t;

#define MAX_ELF_FILE_LENGTH (64)    // max 64 effective lines
#define MAX_ELF_FILE_WIDTH (128)    // max 128 chars per line

typedef struct
{
    char buffer[MAX_ELF_FILE_LENGTH][MAX_ELF_FILE_WIDTH];
    uint64_t line_count;

    uint64_t sht_count;
    sh_entry_t *sht;

    uint64_t symt_count;
    st_entry_t *symt;

    uint64_t reltext_count;
    rl_entry_t *reltext;

    uint64_t reldata_count;
    rl_entry_t *reldata;
} elf_t;

void parse_elf(char *filename, elf_t *elf);
void free_elf(elf_t *elf);
void link_elf(elf_t **srcs, int num_srcs, elf_t *dst);

#endif