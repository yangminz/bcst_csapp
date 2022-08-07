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
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "headers/cpu.h"
#include "headers/memory.h"
#include "headers/interrupt.h"
#include "headers/syscall.h"
#include "headers/process.h"

// from page fault
int copy_physicalframe(pte4_t *child_pte, uint64_t parent_ppn);
int enough_frames(int request_num);
void map_pte4(pte4_t *pte, uint64_t ppn);

static pcb_t *fork_naive_copy(pcb_t *parent_pcb);
static pcb_t *fork_cow(pcb_t *parent_pcb);

uint64_t syscall_fork()
{
    pcb_t *parent = get_current_pcb();
    pcb_t *child = NULL;

#if defined(USE_FORK_NAIVE_COPY)
    child = fork_naive_copy(parent);
#elif defined(USE_FORK_COW)
    child = fork_cow(parent);
#endif

    if (child == NULL)
    {
        return 0;
    }

    return 1;
}

// Implementation

static uint64_t get_newpid()
{
    // find the max pid of current processes
    pcb_t *p = get_current_pcb();
    pcb_t *x = p;
    uint64_t max_pid = p->pid;
    while (x->next != p)
    {
        max_pid = max_pid < x->pid ? x->pid : max_pid;
        x = x->next;
    }
    return max_pid + 1;
}

// Update rax register in user frame
void update_userframe_returnvalue(pcb_t *p, uint64_t retval)
{
    p->context.regs.rax = retval;
    uint64_t uf_vaddr = (uint64_t)p->kstack + KERNEL_STACK_SIZE 
                        - sizeof(trapframe_t) - sizeof(userframe_t);
    userframe_t *uf = (userframe_t *)uf_vaddr;
    uf->regs.rax = retval;
}

static void copy_vmareas(pcb_t *src, pcb_t *dst)
{
#ifdef USE_FORK_COW
    assert(src != NULL && dst != NULL);

    dst->mm.vma.head = 0;
    dst->mm.vma.count = 0;
    dst->mm.vma.update_head = src->mm.vma.update_head;

    if (src->mm.vma.count == 0)
    {
        return;
    }

    vm_area_t *src_vma = (vm_area_t *)src->mm.vma.head;
    for (int i = 0; i < src->mm.vma.count; ++ i)
    {
        vm_area_t *dst_vma = KERNEL_malloc(sizeof(vm_area_t));
        dst_vma->vma_start = src_vma->vma_start;
        dst_vma->vma_end = src_vma->vma_end;
        dst_vma->mode_value = src_vma->mode_value;
        strcpy(dst_vma->filepath, src_vma->filepath);
        dst_vma->rbt_color = src_vma->rbt_color;

        // update the shared bit in vma mode
        src_vma->vma_mode.private = 0;
        dst_vma->vma_mode.private = 0;

        // add the new virtual memory area to child process
        vma_add_area(dst, dst_vma);

        // move to next vma in parent
        src_vma = src_vma->next;
    }

    assert(dst->mm.vma.count == src->mm.vma.count);
#endif
}

static pte123_t *copy_pagetable(pte123_t *src, int level)
{
    // allocate one page for destination
    pte123_t *dst = KERNEL_malloc(sizeof(pte123_t) * PAGE_TABLE_ENTRY_NUM);

    // copy the current page table to destination
    memcpy(dst, src, sizeof(pte123_t) * PAGE_TABLE_ENTRY_NUM);

    if (level == 4)
    {
#ifdef USE_FORK_COW
        // copy on write
        // set the all pte4 to be read only
        for (int j = 0; j < PAGE_TABLE_ENTRY_NUM; ++ j)
        {
            if (dst[j].present == 1)
            {
                // This is a valid pte4
                pte4_t *pte = (pte4_t *)&dst[j];

                // set the page table entry to be read only
                // to trigger protection fault
                pte->readonly = 1;
                (&src[j])->readonly = 1;

                // update page_map.mappings
                uint64_t ppn = (uint64_t)(((pte4_t *)&src[j])->ppn);
                map_pte4((pte4_t *)&dst[j], ppn);
            }
        }
#endif
        return dst;
    }

    // check source
    for (int i = 0; i < PAGE_TABLE_ENTRY_NUM; ++ i)
    {
        if (src[i].present == 1)
        {
            pte123_t *src_next = (pte123_t *)(uint64_t)(src[i].paddr);
            dst[i].paddr = (uint64_t)copy_pagetable(src_next, level + 1);
        }
    }

    return dst;
}

static void copy_userframes(pte123_t *src, pte123_t *dst, int level)
{
    if (level == 4)
    {
        pte4_t *parent_pt = (pte4_t *)src;
        pte4_t *child_pt = (pte4_t *)dst;

        // copy user frames here
        for (int j = 0; j < PAGE_TABLE_ENTRY_NUM; ++ j)
        {
            if (parent_pt[j].present == 1)
            {
                // copy the physical frame to child
                int copy_ = copy_physicalframe(&child_pt[j], parent_pt[j].ppn);
                assert(copy_ == 1);
            }
        }

        return;
    }

    // DFS to go down
    for (int i = 0; i < PAGE_TABLE_ENTRY_NUM; ++ i)
    {
        if (src[i].present == 1)
        {
            pte123_t *src_next = (pte123_t *)(uint64_t)(src[i].paddr);
            pte123_t *dst_next = (pte123_t *)(uint64_t)(dst[i].paddr);
            copy_userframes(src_next, dst_next, level + 1);
        }
    }
}

static int get_childframe_num(pte123_t *p, int level)
{
    int count = 0;

    if (level == 4)
    {
        for (int i = 0; i < PAGE_TABLE_ENTRY_NUM; ++ i)
        {
            if (p[i].present == 1)
            {
                count += 1;
            }
        }
    }
    else
    {
        for (int i = 0; i < PAGE_TABLE_ENTRY_NUM; ++ i)
        {
            if (p[i].present == 1)
            {
                count += get_childframe_num(
                    (pte123_t *)(uint64_t)p[i].paddr, level + 1);
            }
        }
    }

    return count;
}

#define PAGE_TABLE_ENTRY_PADDR_MASK (~((0xffffffffffffffff >> 12) << 12))

static int compare_pagetables(pte123_t *p, pte123_t *q, int level)
{
    for (int i = 0; i < PAGE_TABLE_ENTRY_NUM; ++ i)
    {
        if ((p[i].pte_value & PAGE_TABLE_ENTRY_PADDR_MASK) != 
            (q[i].pte_value & PAGE_TABLE_ENTRY_PADDR_MASK))
        {
            // this entry not match
            assert(0);
        }

        if (level < 4 && p[i].present == 1 &&
            compare_pagetables(
                (pte123_t *)(uint64_t)p[i].paddr, 
                (pte123_t *)(uint64_t)q[i].paddr, level + 1) == 0)
        {
            // sub-pages not match
            assert(0);
        }
        else if (level == 4 && p[i].present == 1)
        {
            // compare the physical frame content
            int p_ppn = p[i].paddr;
            int q_ppn = q[i].paddr;
            assert(memcmp(&pm[p_ppn << 12], &pm[q_ppn << 12], PAGE_SIZE) == 0);
        }

    }

    return 1;
}

static pcb_t *copy_pcb(pcb_t *parent_pcb)
{
    // Note that all system calls are actually compiled to binaries
    // Operating system itself is a binary, a software, executed by CPU
    // Here we use C language to write the fork
    // But actually, we should write instructions:
    // mov_handler
    // add_handler, etc.
    //
    // Now, let's start to write `fork`

    // ATTENTION HERE!!!
    // In a realistic OS, you need to allocate kernel pages as well
    // And then copy the data on parent kernel page to child's.
    pcb_t *child_pcb = KERNEL_malloc(sizeof(pcb_t));
    if (child_pcb == NULL)
    {
        return NULL;
    }
    memcpy(child_pcb, parent_pcb, sizeof(pcb_t));

    // update child PID
    child_pcb->pid = get_newpid();

    // COW optimize: create a kernel stack for child process
    // NOTE KERNEL STACK MUST BE ALIGNED
    kstack_t *child_kstack = aligned_alloc(KERNEL_STACK_SIZE, KERNEL_STACK_SIZE);
    memcpy(child_kstack, (kstack_t *)parent_pcb->kstack, sizeof(kstack_t));
    child_pcb->kstack = child_kstack;
    child_kstack->threadinfo.pcb = child_pcb;

    // prepare child process context (especially RSP) for context switching
    // for child process to be switched to
    child_pcb->context.regs.rsp = (uint64_t)child_kstack + KERNEL_STACK_SIZE -
                                    sizeof(trapframe_t) - sizeof(userframe_t);

    // add child process PCB to linked list for scheduling
    // it's better the child process is right after parent
    if (parent_pcb->prev == parent_pcb)
    {
        // parent is the only PCB in the linked list
        assert(parent_pcb->next == parent_pcb);
        parent_pcb->prev = child_pcb;
    }
    parent_pcb->next = child_pcb;
    child_pcb->prev = parent_pcb;

    // Fork's secret:
    // call once, return twice
    update_userframe_returnvalue(parent_pcb, child_pcb->pid);
    update_userframe_returnvalue(child_pcb, 0);

    // copy the entire page table of parent
    child_pcb->mm.pgd = copy_pagetable(parent_pcb->mm.pgd, 1);

    // copy virtual memory areas
    copy_vmareas(parent_pcb, child_pcb);

    // flush TLB
#if defined(USE_TLB_HARDWARE) && defined(USE_PAGETABLE_VA2PA)
    flush_tlb();
#endif

    // All copy works are done here
    return child_pcb;
}

static pcb_t *fork_naive_copy(pcb_t *parent_pcb)
{
    // check memory size because we directly copy the physical frames
    int needed_userframe_num = get_childframe_num(parent_pcb->mm.pgd, 1);
    if (enough_frames(needed_userframe_num) == 0)
    {
        // there are no enough frames for child process to use
        update_userframe_returnvalue(parent_pcb, -1);
        return NULL;
    }

    // copy PCB
    pcb_t *child_pcb = copy_pcb(parent_pcb);
    assert(child_pcb != NULL);

    // directly copy user frames in main memory now
    copy_userframes(parent_pcb->mm.pgd, child_pcb->mm.pgd, 1);
    assert(compare_pagetables(child_pcb->mm.pgd, parent_pcb->mm.pgd, 1) == 1);

    return child_pcb;
}

static pcb_t *fork_cow(pcb_t *parent_pcb)
{
    pcb_t *child_pcb = copy_pcb(parent_pcb);
    assert(child_pcb != NULL);

    return child_pcb;
}