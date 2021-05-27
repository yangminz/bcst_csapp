/* BCST - Introduction to Computer Systems
 * Author:      yangminz@outlook.com
 * Github:      https://github.com/yangminz/bcst_csapp
 * Bilibili:    https://space.bilibili.com/4564101
 * Zhihu:       https://www.zhihu.com/people/zhao-yang-min
 * This project (code repository and videos) is exclusively owned by yangminz 
 * and shall not be used for commercial and profitting purpose 
 * without yangminz's permission.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "headers/linker.h"
#include "headers/common.h"

void parse_elf(char *filename, elf_t *elf);

int main()
{
    elf_t src[2];
    parse_elf("./files/exe/sum.elf.txt", &src[0]);
    parse_elf("./files/exe/main.elf.txt", &src[1]);

    elf_t dst;
    elf_t *srcp[2];
    srcp[0] = &src[0];
    srcp[1] = &src[1];
    link_elf((elf_t **)&srcp, 2, &dst);

    write_eof("./files/exe/output.eof.txt", &dst);

    free_elf(&src[0]);
    free_elf(&src[1]);
    free_elf(&dst);

    finally_cleanup();

    return 0;
}