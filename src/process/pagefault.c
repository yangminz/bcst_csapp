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


void kernel_pagefault_handler(address_t vaddr)
{
    /*
    // get page table directory from rsp

    uint64_t rsp_now = cpu_reg.rsp;
    uint64_t kstack_top = (rsp_now >> 13) << 13;
    uint64_t kstack_bottom = kstack_top - KERNEL_STACK_SIZE;
    
    kstack_t *kstack = (kstack_t *)kstack_bottom;
    uint64_t pgd_paddr = kstack->threadinfo.pcb->mm.pgd_paddr;

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
);

    // load page from disk to physical memory first
    daddr = pte->daddr;
    swap_in(daddr, ppn
    pte->pte_value = 0;
    pte->present = 1;
    pte->ppn = ppn;
    pte->dirty = 0;

    page_map[ppn].allocated = 1;
    page_map[ppn].time = 0;
    page_map[ppn].dirty = 0;
    page_map[ppn].pte4 = pte;
    page_map[ppn].daddr = daddr;
    */
}