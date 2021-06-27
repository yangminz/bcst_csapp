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

static uint64_t page_walk(uint64_t vaddr_value);

// consider this function va2pa as functional
uint64_t va2pa(uint64_t vaddr)
{
    // use page table as va2pa
    return page_walk(vaddr);
}

// input - virtual address
// output - physical address
static uint64_t page_walk(uint64_t vaddr_value)
{
    address_t vaddr = {
        .vaddr_value = vaddr_value
    };
    int page_table_size = PAGE_TABLE_ENTRY_NUM * sizeof(pte123_t);  // should be 4KB

    // CR3 register's value is malloced on the heap of the simulator
    pte123_t *pgd = (pte123_t *)cpu_controls.cr3;
    assert(pgd != NULL);
    
    if (pgd[vaddr.vpn1].present == 1)
    {
        // PHYSICAL PAGE NUMBER of the next level page table
        // aka. high bits starting address of the page table
        pte123_t *pud = pgd[vaddr.vpn1].paddr;

        if (pud[vaddr.vpn2].present == 1)
        {
            // find pmd ppn

            pte123_t *pmd = (pte123_t *)(pud[vaddr.vpn2].paddr);

            if (pmd[vaddr.vpn3].present == 1)
            {
                // find pt ppn
                
                pte4_t *pt = (pte4_t *)(pmd[vaddr.vpn3].paddr);

                if (pt[vaddr.vpn4].present == 1)
                {
                    // find page table entry
                    address_t paddr = {
                        .ppn = pt[vaddr.vpn4].ppn,
                        .ppo = vaddr.vpo    // page offset inside the 4KB page
                    };

                    return paddr.paddr_value;
                }
                else
                {
                    // page table entry not exists
#ifdef DBUEG_PAGE_WALK
                    printf("page walk level 4: pt[%lx].present == 0\n\tmalloc new page table for it\n", vaddr.vpn1);
#endif
                    pte4_t *pt = malloc(page_table_size);
                    memset(pt, 0, page_table_size);

                    // set page table entry
                    pmd[vaddr.vpn3].present = 1;
                    pud[vaddr.vpn3].paddr   = (uint64_t)pt;

                    // TODO: page fault here
                    // map the physical page and the virtual page
                    exit(0);
                }
            }
            else
            {
                // pt - level 4 not exists
#ifdef DBUEG_PAGE_WALK
                printf("page walk level 3: pmd[%lx].present == 0\n\tmalloc new page table for it\n", vaddr.vpn1);
#endif
                pte4_t *pt = malloc(page_table_size);
                memset(pt, 0, page_table_size);

                // set page table entry
                pmd[vaddr.vpn3].present = 1;
                pud[vaddr.vpn3].paddr   = (uint64_t)pt;

                // TODO: page fault here
                // map the physical page and the virtual page
                exit(0);
            }
        }
        else
        {
            // pmd - level 3 not exists
#ifdef DBUEG_PAGE_WALK
            printf("page walk level 2: pud[%lx].present == 0\n\tmalloc new page table for it\n", vaddr.vpn1);
#endif
            pte123_t *pmd = malloc(page_table_size);
            memset(pmd, 0, page_table_size);

            // set page table entry
            pud[vaddr.vpn2].present = 1;
            pud[vaddr.vpn2].paddr   = (uint64_t)pmd;

            // TODO: page fault here
            // map the physical page and the virtual page
            exit(0);
        }
    }
    else
    {
        // pud - level 2 not exists
#ifdef DBUEG_PAGE_WALK
        printf("page walk level 1: pgd[%lx].present == 0\n\tmalloc new page table for it\n", vaddr.vpn1);
#endif
        pte123_t *pud = malloc(page_table_size);
        memset(pud, 0, page_table_size);

        // set page table entry
        pgd[vaddr.vpn1].present = 1;
        pgd[vaddr.vpn1].paddr   = (uint64_t)pud;

        // TODO: page fault here
        // map the physical page and the virtual page
        exit(0);
    }
}