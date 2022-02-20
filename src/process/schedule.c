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
#include "headers/process.h"

static pcb_t *get_current_pcb()
{
    uint64_t rsp = cpu_reg.rsp;
    uint64_t kstack_bottom_vaddr = ((rsp >> 13) << 13);
    kstack_t *ks = (kstack_t *)kstack_bottom_vaddr;
    pcb_t *current_pcb = ks->threadinfo.pcb;

    return current_pcb;
}

static void push_context()
{
    // check the size
    uint32_t ctx_size = sizeof(context_t);
    assert(ctx_size < KERNEL_STACK_SIZE);

    // get the range of kernel stack
    uint64_t rsp = cpu_reg.rsp;
    uint64_t kstack_bottom_vaddr = ((rsp >> 13) << 13);
    uint64_t kstack_top_vaddr = kstack_bottom_vaddr + KERNEL_STACK_SIZE;
    assert(kstack_bottom_vaddr <= rsp && rsp < kstack_top_vaddr);
    assert(kstack_bottom_vaddr <= rsp - ctx_size);

    // store user frame to kstack
    rsp -= ctx_size;
    context_t ctx = {
        .general_registers = cpu_reg,
        .flags = cpu_flags
    };
    memcpy((userframe_t *)rsp, &ctx, ctx_size);

    // push RSP
    cpu_reg.rsp = rsp;

    // store the current rsp to rsp field of PCB
    pcb_t *old_pcb = get_current_pcb();
    old_pcb->ctx_rsp = cpu_reg.rsp;
}

static void pop_context()
{
    // check the size
    uint32_t ctx_size = sizeof(context_t);
    assert(ctx_size < KERNEL_STACK_SIZE);

    // get the vaddr of kstack low
    uint64_t rsp = cpu_reg.rsp;
    uint64_t kstack_bottom_vaddr = ((rsp >> 13) << 13);
    uint64_t kstack_top_vaddr = kstack_bottom_vaddr + KERNEL_STACK_SIZE;
    assert(kstack_bottom_vaddr <= rsp && rsp < kstack_top_vaddr);
    assert(rsp + ctx_size < kstack_top_vaddr);

    // restore user frame from kstack
    context_t ctx;
    memcpy(&ctx, (context_t *)rsp, ctx_size);
    rsp += ctx_size;
    
    // restore cpu registers from user frame
    memcpy(&cpu_reg, &ctx.general_registers, sizeof(cpu_reg));
    memcpy(&cpu_flags, &ctx.flags, sizeof(cpu_flags));

    // pop rsp
    cpu_reg.rsp = rsp;
}

void os_schedule()
{
    printf("\t\tOS schedule\n");

    // The magic is: RIP is not updated at all
    // only kstack & page table will do the switch

    uint64_t rsp = cpu_reg.rsp;
    uint64_t kstack_bottom_vaddr = ((rsp >> 13) << 13);

    kstack_t *kstack_old = (kstack_t *)kstack_bottom_vaddr;
    pcb_t *pcb_old = kstack_old->threadinfo.pcb;

    // pcb_new should be selected by the scheduling algorithm
    pcb_t *pcb_new = pcb_old->next;

    // context switch

    // store the context of the old process
    push_context();

    cpu_reg.rsp = pcb_new->ctx_rsp;

    // restore the context of the new process
    pop_context();

    // TODO: update TR -> TSS

    // update CR3 -> page table in MMU
    cpu_controls.cr3 = pcb_new->mm.pgd_paddr;
}