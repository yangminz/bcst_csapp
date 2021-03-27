#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<headers/linker.h>
#include<headers/common.h>

int read_elf(const char *filename, uint64_t bufaddr);
int parse_table_entry(char *str, char ***ent);


void parse_sh(char *str, sh_entry_t *sh);
void free_table_entry(char **ent, int n);

void print_sh_entry(sh_entry_t *sh);

void parse_elf(char *filename, elf_t *elf);
void free_elf(elf_t *elf);

int main()
{
    elf_t src[2];

    parse_elf("./files/exe/sum.elf.txt", &src[0]);
    parse_elf("./files/exe/main.elf.txt", &src[1]);

    free_elf(&src[0]);
    free_elf(&src[1]);
    
    return 0;
}