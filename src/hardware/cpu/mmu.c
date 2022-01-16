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
                    // search paddr from main memory and disk
                    // TODO: raise exception 14 (page fault) here
                    // switch privilege from user mode (ring 3) to kernel mode (ring 0)
                    page_fault_handler(&pt[vaddr.vpn4], vaddr);

                    /*
                    pte4_t *pt = malloc(page_table_size);
                    memset(pt, 0, page_table_size);

                    // set page table entry
                    pmd[vaddr.vpn3].present = 1;
                    pud[vaddr.vpn3].paddr   = (uint64_t)pt;

                    // TODO: page fault here
                    // map the physical page and the virtual page
                    exit(0);
                    */
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

static void page_fault_handler(pte4_t *pte, address_t vaddr)
{
    // select one victim physical page to swap to disk
    assert(pte->present == 0);

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
#endif