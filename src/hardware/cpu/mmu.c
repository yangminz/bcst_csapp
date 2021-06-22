/* BCST - Introduction to Computer Systems
 * Author:      yangminz@outlook.com
 * Github:      https://github.com/yangminz/bcst_csapp
 * Bilibili:    https://space.bilibili.com/4564101
 * Zhihu:       https://www.zhihu.com/people/zhao-yang-min
 * This project (code repository and videos) is exclusively owned by yangminz 
 * and shall not be used for commercial and profitting purpose 
 * without yangminz's permission.
 */

// Memory Management Unit 
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "headers/cpu.h"
#include "headers/memory.h"
#include "headers/common.h"
#include "headers/address.h"

// consider this function va2pa as functional
uint64_t va2pa(uint64_t vaddr)
{
    // use page table as va2pa
}

// input - virtual address
// output - physical address
static uint64_t page_walk(uint64_t vaddr_value)
{
    address_t vaddr = {
        .vaddr_value = vaddr_value
    };

    // CR3 register's value is malloced on the heap of the simulator
    pte123_t *pgd = (pte123_t *)cpu_controls.cr3;

    pte123_t *pmd = (pte123_t *)(pgd[vaddr.vpn1].paddr);
}