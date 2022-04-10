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
int swap_in(uint64_t saddr, uint64_t ppn);
int swap_out(uint64_t saddr, uint64_t ppn);

// physical page descriptor
typedef struct
{
    int allocated;
    int dirty;
    int time;   // LRU cache: 0 - Fresh

    // real world: mapping to anon_vma or address_space
    // we simply the situation here
    // TODO: if multiple processes are using this page? E.g. Shared library
    pte4_t *pte4;       // the reversed mapping: from PPN to page table entry
    uint64_t saddr;   // binding the revesed mapping with mapping to disk
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
            pte123_t *new_tab = (pte123_t *)KERNEL_malloc(PAGE_TABLE_ENTRY_NUM * sizeof(pte123_t));
            
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

void page_map_init()
{
    for (int k = 0; k < MAX_NUM_PHYSICAL_PAGE; ++ k)
    {
        page_map[k].allocated = 0;
        page_map[k].dirty = 0;
        page_map[k].time = 0;
        page_map[k].pte4 = NULL;
    }
}

void pagemap_update_time(uint64_t ppn)
{
    assert(0 <= ppn && ppn < MAX_NUM_PHYSICAL_PAGE);
    assert(page_map[ppn].allocated == 1);
    assert(page_map[ppn].pte4->present == 1);
    for (int i = 0; i < MAX_NUM_PHYSICAL_PAGE; ++ i)
    {
        page_map[i].time += 1;
    }
    page_map[ppn].time = 0;
}

void pagemap_dirty(uint64_t ppn)
{
    assert(0 <= ppn && ppn < MAX_NUM_PHYSICAL_PAGE);
    assert(page_map[ppn].allocated == 1);
    assert(page_map[ppn].pte4->present == 1);
    page_map[ppn].dirty = 1;
    page_map[ppn].pte4->dirty = 1;
}

// used by frame swap-in from swap space
// for newly allocated anoymous page
void set_pagemap_swapaddr(uint64_t ppn, uint64_t swap_address)
{
    assert(0 <= ppn && ppn < MAX_NUM_PHYSICAL_PAGE);
    page_map[ppn].saddr = swap_address;
}

void map_pte4(pte4_t *pte, uint64_t ppn)
{
    assert(0 <= ppn && ppn < MAX_NUM_PHYSICAL_PAGE);
    // must use an empty reversed mapping slot
    assert(page_map[ppn].allocated == 0);
    assert(page_map[ppn].dirty == 0);
    assert(page_map[ppn].pte4 == NULL);

    // Let's consider this, where can we store the swap address on disk?
    // In this case of physical page being allocated and mapped,
    // the swap address is stored in reversed mapping array
    page_map[ppn].saddr = pte->saddr;

    // map the level 4 page table
    pte->present = 1;
    pte->ppn = ppn;
    pte->dirty = 0;

    // reversed mapping
    page_map[ppn].allocated = 1;    // allocated for vaddr
    page_map[ppn].dirty = 0;        // allocated as clean
    page_map[ppn].time = 0;         // most recently used physical page
    page_map[ppn].pte4 = pte;

    /*  When mapped
        Page table entry: present = 1, ppn
        page_map[ppn]: pte4, swap address
        data on DRAM[ppn] (DIRTY/CLEAN), SWAP[swap address]
     */
}

void unmap_pte4(uint64_t ppn)
{
    assert(0 <= ppn && ppn < MAX_NUM_PHYSICAL_PAGE);
    // Get the page table entry from reversed mapping array by ppn
    // Note that in this case the page MUST be allocated
    assert(page_map[ppn].allocated == 1);
    pte4_t *pte = page_map[ppn].pte4;
    assert(pte->present == 1);

    pte->pte_value = 0;
    pte->present = 0;
    // In this case, page_map[ppn] would be mapped by other page table.
    // Previously, this is used to store the swap address.
    // Now we need to move the swap address to the page table entry.
    pte->saddr = page_map[ppn].saddr;

    // clear the reversed mapping
    page_map[ppn].allocated = 0;
    page_map[ppn].dirty = 0;
    page_map[ppn].time = 0;
    page_map[ppn].pte4 = NULL;

    /*  When unmapped
        Page table entry: present = 0, swap address
        page_map[ppn]: not applicable any more
        data on SWAP[swap address] (WB)
     */
    // now page_map[ppn] can be used by other page table entry
}

void fix_pagefault()
{
    // get page table directory from rsp
    pcb_t *pcb = get_current_pcb();
    // same as what is stored in CR3 register exactly
    pte123_t *pgd = pcb->mm.pgd;

    // get the faulting address from MMU register
    address_t vaddr = {.address_value = mmu_vaddr_pagefault};

    // get the level 4 page table entry
    pte4_t *pte = get_entry4(pgd, &vaddr);
    
    // 1. try to request one free physical page from DRAM
    // kernel's responsibility
    for (int i = 0; i < MAX_NUM_PHYSICAL_PAGE; ++ i)
    {
        if (page_map[i].allocated == 0)
        {
            // found i as free ppn
            map_pte4(pte, i);
         
            printf("\033[34;1m\tPageFault: use free ppn %d\033[0m\n", i);
            return;
        }
    }

    // 2. no free physical page: select one clean page (LRU) and overwrite
    // in this case, there is no DRAM - DISK transaction
    // you know you can optimize this loop in the previous one.
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
    
    // this is the selected ppn for vaddr
    if (-1 != lru_ppn && lru_ppn < MAX_NUM_PHYSICAL_PAGE)
    {
        // reversed mapping will find the victim page table
        // unmap the victim (LRU)
        unmap_pte4(lru_ppn);

        // load page from disk to physical memory
        // at the victim's ppn
        swap_in(pte->saddr, lru_ppn);
        map_pte4(pte, lru_ppn);

        printf("\033[34;1m\tPageFault: discard clean ppn %d as victim\033[0m\n", lru_ppn);
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

    // write back
    swap_out(page_map[lru_ppn].saddr, lru_ppn);

    // unmap victim
    unmap_pte4(lru_ppn);

    // load page from disk to physical memory
    swap_in(pte->saddr, lru_ppn);
    map_pte4(pte, lru_ppn);

    printf("\033[34;1m\tPageFault: write back & use ppn %d\033[0m\n", lru_ppn);
}