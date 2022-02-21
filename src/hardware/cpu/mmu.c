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
#include "headers/interrupt.h"

// -------------------------------------------- //
// TLB cache struct
// -------------------------------------------- //

#define NUM_TLB_CACHE_LINE_PER_SET (8)

typedef struct 
{
    int valid;
    uint64_t tag;
    uint64_t ppn;
} tlb_cacheline_t;

typedef struct
{
    tlb_cacheline_t lines[NUM_TLB_CACHE_LINE_PER_SET];
} tlb_cacheset_t;

typedef struct
{
    tlb_cacheset_t sets[(1 << TLB_CACHE_INDEX_LENGTH)];
} tlb_cache_t;

static tlb_cache_t mmu_tlb;

static uint64_t page_walk(uint64_t vaddr_value);
static void page_fault_handler(pte4_t *pte, address_t vaddr);

static int read_tlb(uint64_t vaddr_value, uint64_t *paddr_value_ptr,
    int *free_tlb_line_index);
static int write_tlb(uint64_t vaddr_value, uint64_t paddr_value, 
    int free_tlb_line_index);

int swap_in(uint64_t daddr, uint64_t ppn);
int swap_out(uint64_t daddr, uint64_t ppn);

// consider this function va2pa as functional
uint64_t va2pa(uint64_t vaddr)
{
#ifdef USE_NAVIE_VA2PA
    return vaddr % PHYSICAL_MEMORY_SPACE;
#endif
    uint64_t paddr = 0;

#ifdef USE_TLB_HARDWARE
    int free_tlb_line_index = -1;
    int tlb_hit = read_tlb(vaddr, &paddr, &free_tlb_line_index);

    // TODO: add flag to read tlb failed
    if (tlb_hit)
    {
        // TLB read hit
        return paddr;
    }

    // TLB read miss
#endif

#ifdef USE_PAGETABLE_VA2PA
    // assume that page_walk is consuming much time
    paddr = page_walk(vaddr);
#endif

#ifdef USE_TLB_HARDWARE
    // refresh TLB
    // TODO: check if this paddr from page table is a legal address
    if (paddr != 0)
    {
        // TLB write
        if (write_tlb(vaddr, paddr, free_tlb_line_index) == 1)
        {
            return paddr;
        }
    }
#endif

    // use page table as va2pa
    return paddr;
}

#ifdef USE_TLB_HARDWARE
static int read_tlb(uint64_t vaddr_value, uint64_t *paddr_value_ptr, 
    int *free_tlb_line_index)
{
    address_t vaddr = {
        .address_value = vaddr_value
    };

    tlb_cacheset_t *set = &mmu_tlb.sets[vaddr.tlbi];
    *free_tlb_line_index = -1;

    for (int i = 0; i < NUM_TLB_CACHE_LINE_PER_SET; ++ i)
    {
        tlb_cacheline_t *line = &set->lines[i];

        if (line->valid == 0)
        {
            *free_tlb_line_index = i;
        }

        if (line->tag == vaddr.tlbt &&
            line->valid == 1)
        {
            // TLB read hit
            *paddr_value_ptr = line->ppn;
            return 1;
        }
    }

    // TLB read miss
    *paddr_value_ptr = NULL;
    return 0;
}

static int write_tlb(uint64_t vaddr_value, uint64_t paddr_value, 
    int free_tlb_line_index)
{
    address_t vaddr = {
        .address_value = vaddr_value
    };

    address_t paddr = {
        .address_value = paddr_value
    };

    tlb_cacheset_t *set = &mmu_tlb.sets[vaddr.tlbi];

    if (0 <= free_tlb_line_index && free_tlb_line_index < NUM_TLB_CACHE_LINE_PER_SET)
    {
        tlb_cacheline_t *line = &set->lines[free_tlb_line_index];

        line->valid = 1;
        line->ppn = paddr.ppn;
        line->tag = vaddr.tlbt;

        return 1;
    }

    // no free TLB cache line, select one RANDOM victim
    int random_victim_index = random() % NUM_TLB_CACHE_LINE_PER_SET;

    tlb_cacheline_t *line = &set->lines[random_victim_index];

    line->valid = 1;
    line->ppn = paddr.ppn;
    line->tag = vaddr.tlbt;

    return 1;
}
#endif

#ifdef USE_PAGETABLE_VA2PA
// input - virtual address
// output - physical address
static uint64_t page_walk(uint64_t vaddr_value)
{
    // parse address
    address_t vaddr = {
        .vaddr_value = vaddr_value
    };
    int vpn1 = vaddr.vpn1;
    int vpn2 = vaddr.vpn2;
    int vpn3 = vaddr.vpn3;
    int vpn4 = vaddr.vpn4;
    int vpo = vaddr.vpo;

    int page_table_size = PAGE_TABLE_ENTRY_NUM * sizeof(pte123_t);  // should be 4KB

    // CR3 register's value is malloced on the heap of the simulator
    pte123_t *pgd = (pte123_t *)cpu_controls.cr3;
    assert(pgd != NULL);
    
    if (pgd[vpn1].present == 1)
    {
        // PHYSICAL PAGE NUMBER of the next level page table
        // aka. high bits starting address of the page table
        pte123_t *pud = (pte123_t *)((uint64_t)(pgd[vpn1].paddr));

        if (pud[vpn2].present == 1)
        {
            // find pmd ppn
            pte123_t *pmd = (pte123_t *)((uint64_t)(pud[vpn2].paddr));

            if (pmd[vpn3].present == 1)
            {
                // find pt ppn
                pte4_t *pt = (pte4_t *)((uint64_t)(pmd[vpn3].paddr));

                if (pt[vpn4].present == 1)
                {
                    // find page table entry
                    address_t paddr = {
                        .ppn = pt[vpn4].ppn,
                        .ppo = vpo    // page offset inside the 4KB page
                    };
                    return paddr.paddr_value;
                }
                else
                {
                    // page table entry not exists
#ifdef DEBUG_PAGE_WALK
                    printf("page walk (%lx): level 4 page fault: pt[%lx].present == 0\n", vaddr_value, vpn4);
#endif
                }
            }
            else
            {
                // pt - level 4 not exists
#ifdef DEBUG_PAGE_WALK
                printf("page walk (%lx): level 3 page fault: pmd[%lx].present == 0\n", vaddr_value, vpn3);
#endif
            }
        }
        else
        {
            // pmd - level 3 not exists
#ifdef DEBUG_PAGE_WALK
            printf("page walk (%lx): level 2 page fault: pud[%lx].present == 0\n", vaddr_value, vpn2);
#endif
        }
    }
    else
    {
        // pud - level 2 not exists
#ifdef DEBUG_PAGE_WALK
        printf("page walk (%lx): level 1 page fault: pgd[%lx].present == 0\n", vaddr_value, vpn1);
#endif
    }

    mmu_vaddr_pagefault = vaddr.vaddr_value;
    interrupt_stack_switching(0x80);
    return 0;
}
#endif