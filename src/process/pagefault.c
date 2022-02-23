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
#include <stdlib.h>
#include <assert.h>
#include "headers/cpu.h"
#include "headers/memory.h"
#include "headers/common.h"
#include "headers/address.h"
#include "headers/interrupt.h"
#include "headers/process.h"

// search paddr from main memory and disk
// TODO: raise exception 14 (page fault) here
// switch privilege from user mode (ring 3) to kernel mode (ring 0)
// page_fault_handler(&pt[vaddr.vpn4], vaddr);

// swap in/out
int swap_in(uint64_t daddr, uint64_t ppn);
int swap_out(uint64_t daddr, uint64_t ppn);

// 4KB
static int page_table_size = PAGE_TABLE_ENTRY_NUM * sizeof(pte123_t);

// physical page descriptor
typedef struct
{
    int allocated;
    int dirty;
    int time;   // LRU cache

    // real world: mapping to anon_vma or address_space
    // we simply the situation here
    // TODO: if multiple processes are using this page? E.g. Shared library
    pte4_t *pte4;       // the reversed mapping: from PPN to page table entry
    uint64_t daddr;   // binding the revesed mapping with mapping to disk
} pd_t;

// for each pagable (swappable) physical page
// create one reversed mapping
static pd_t page_map[MAX_NUM_PHYSICAL_PAGE]; 

// get the level 4 page table entry
static pte4_t *get_entry4(pte123_t *pgd, address_t *vaddr)
{
    int vpns[4] = {
        vaddr->vpn1,
        vaddr->vpn2,
        vaddr->vpn3,
        vaddr->vpn4,
    };

    assert(pgd != NULL);
    assert(sizeof(pte123_t) == sizeof(pte4_t));

    int level = 0;
    pte123_t *tab = pgd;
    while (level < 4)
    {
        int vpn = vpns[level];
        if (tab[vpn].present != 1)
        {
            // allocate a new page for next level

            // note that this is a 48-bit address !!!
            // the high bits are all zero
            // And sizeof(pte123_t) == sizeof(pte4_t)
            pte123_t *new_tab = (pte123_t *)malloc(PAGE_TABLE_ENTRY_NUM * sizeof(pte123_t));
            
            // .paddr field is 50 bits
            tab[vpn].paddr = (uint64_t)new_tab;
            tab[vpn].present = 1;
        }

        // move to next level
        tab = (pte123_t *)((uint64_t)tab[vpn].paddr);
        level += 1;
    }

    pte4_t *pt = (pte4_t *)tab;
    return &pt[vaddr->vpn4];
}

void fix_pagefault()
{
    // get page table directory from rsp
    pcb_t *pcb = get_current_pcb();
    pte123_t *pgd = pcb->mm.pgd;

    // get the faulting address from MMU register
    address_t vaddr = {.address_value = mmu_vaddr_pagefault};

    // get the level 4 page table entry
    pte4_t *pte = get_entry4(pgd, &vaddr);
    
    // this is the selected ppn for vaddr
    int ppn = -1;
    pte4_t *victim = NULL;
    uint64_t daddr = 0xffffffffffffffff;

    // 1. try to request one free physical page from DRAM
    // kernel's responsibility
    for (int i = 0; i < MAX_NUM_PHYSICAL_PAGE; ++ i)
    {
        if (page_map[i].pte4->present == 0)
        {
            printf("PageFault: use free ppn %d\n", i);

            // found i as free ppn
            ppn = i;

            page_map[ppn].allocated = 1;    // allocate for vaddr
            page_map[ppn].dirty = 0;    // allocated as clean
            page_map[ppn].time = 0;    // most recently used physical page
            page_map[ppn].pte4 = pte;

            pte->present = 1;
            pte->ppn = ppn;
            pte->dirty = 0;

            return;
        }
    }

    // 2. no free physical page: select one clean page (LRU) and overwrite
    // in this case, there is no DRAM - DISK transaction
    int lru_ppn = -1;
    int lru_time = -1;
    for (int i = 0; i < MAX_NUM_PHYSICAL_PAGE; ++ i)
    {
        if (page_map[i].dirty == 0 && 
            lru_time < page_map[i].time)
        {
            lru_time = page_map[i].time;
            lru_ppn = i;
        }
    }
    
    if (-1 != lru_ppn && lru_ppn < MAX_NUM_PHYSICAL_PAGE)
    {
        ppn = lru_ppn;

        // reversed mapping
        victim = page_map[ppn].pte4;

        victim->pte_value = 0;
        victim->present = 0;
        victim->daddr = page_map[ppn].daddr;

        // load page from disk to physical memory first
        daddr = pte->daddr;
        swap_in(pte->daddr, ppn);

        pte->pte_value = 0;
        pte->present = 1;
        pte->ppn = ppn;
        pte->dirty = 0;

        page_map[ppn].allocated = 1;
        page_map[ppn].time = 0;
        page_map[ppn].dirty = 0;
        page_map[ppn].pte4 = pte;
        page_map[ppn].daddr = daddr;

        return;
    }

    // 3. no free nor clean physical page: select one LRU victim
    // write back (swap out) the DIRTY victim to disk
    lru_ppn = -1;
    lru_time = -1;
    for (int i = 0; i < MAX_NUM_PHYSICAL_PAGE; ++ i)
    {
        if (lru_time < page_map[i].time)
        {
            lru_time = page_map[i].time;
            lru_ppn = i;
        }
    }

    assert(0 <= lru_ppn && lru_ppn < MAX_NUM_PHYSICAL_PAGE);

    ppn = lru_ppn;

    // reversed mapping
    victim = page_map[ppn].pte4;

    // write back
    swap_out(page_map[ppn].daddr, ppn);

    victim->pte_value = 0;
    victim->present = 0;
    victim->daddr = page_map[ppn].daddr;

    // load page from disk to physical memory first
    daddr = pte->daddr;
    swap_in(daddr, ppn);
    pte->pte_value = 0;
    pte->present = 1;
    pte->ppn = ppn;
    pte->dirty = 0;

    page_map[ppn].allocated = 1;
    page_map[ppn].time = 0;
    page_map[ppn].dirty = 0;
    page_map[ppn].pte4 = pte;
    page_map[ppn].daddr = daddr;
}