#include<stdio.h>
#include<stdint.h>
#include<stdlib.h>
#include<headers/linker.h>
#include<headers/common.h>

int read_elf(const char *filename, uint64_t bufaddr);

int main()
{
    char buf[MAX_ELF_FILE_LENGTH][MAX_ELF_FILE_WIDTH];

    int count = read_elf("./files/exe/sum.elf.txt", (uint64_t)&buf);
    for (int i = 0; i < count; ++ i)
    {
        printf("%s\n", buf[i]);
    }
    return 0;
}