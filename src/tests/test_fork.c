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
#include "headers/cpu.h"
#include "headers/memory.h"
#include "headers/common.h"
#include "headers/algorithm.h"
#include "headers/address.h"
#include "headers/instruction.h"
#include "headers/interrupt.h"
#include "headers/process.h"

void map_pte4(pte4_t *pte, uint64_t ppn);
void unmap_pte4(uint64_t ppn);
void page_map_init();
void pagemap_dirty(uint64_t ppn);
void pagemap_update_time(uint64_t ppn);
void set_pagemap_swapaddr(uint64_t ppn, uint64_t swap_address);
uint64_t allocate_swappage(uint64_t ppn);

static void link_page_table(pte123_t *pgd, pte123_t *pud, pte123_t *pmd, pte4_t *pt,
    int ppn, address_t *vaddr)
{
    int vpn1 = vaddr->vpn1;
    int vpn2 = vaddr->vpn2;
    int vpn3 = vaddr->vpn3;
    int vpn4 = vaddr->vpn4;

    (&(pgd[vpn1]))->paddr = (uint64_t)&pud[0];
    (&(pgd[vpn1]))->present = 1;

    (&(pud[vpn2]))->paddr = (uint64_t)&pmd[0];
    (&(pud[vpn2]))->present = 1;

    (&(pmd[vpn3]))->paddr = (uint64_t)&pt[0];
    (&(pmd[vpn3]))->present = 1;

    (&(pt[vpn4]))->ppn = ppn;
    (&(pt[vpn4]))->present = 1;

    map_pte4(&(pt[vpn4]), ppn);
}

static void TestFork()
{
    printf("================\nTesting fork ...\n");

    cpu_reg.rsp = 0x7ffffffee0f0;
    cpu_pc.rip = 0x00400000;
    address_t code_addr = {.address_value = cpu_pc.rip};
    address_t stack_addr = {.address_value = cpu_reg.rsp};
    
    page_map_init();

    // pcb is needed to trigger page fault
    pcb_t p1;
    memset(&p1, 0, sizeof(pcb_t));
    p1.pid = 1;
    // the next switched process would still be p1
    p1.next = &p1;
    p1.prev = &p1;

    // prepare PGD
    pte123_t p1_pgd[512];
    memset(&p1_pgd, 0, sizeof(pte123_t) * 512);
    p1.mm.pgd = &p1_pgd[0];

    // prepare code page tables
    pte123_t p1_pud[512];
    pte123_t p1_pmd[512];
    pte4_t   p1_pt[512];
    memset(&p1_pud, 0, sizeof(pte123_t) * 512);
    memset(&p1_pmd, 0, sizeof(pte123_t) * 512);
    memset(&p1_pt, 0, sizeof(pte123_t) * 512);
    link_page_table(&p1_pgd[0], &p1_pud[0], &p1_pmd[0], &p1_pt[0], 0, &code_addr);

    // prepare user mode stack frame
    pte123_t p1_pud_stack[512];
    pte123_t p1_pmd_stack[512];
    pte4_t   p1_pt_stack[512];
    memset(&p1_pud_stack, 0, sizeof(pte123_t) * 512);
    memset(&p1_pmd_stack, 0, sizeof(pte123_t) * 512);
    memset(&p1_pt_stack, 0, sizeof(pte123_t) * 512);
    link_page_table(&p1_pgd[0], &p1_pud_stack[0], &p1_pmd_stack[0], &p1_pt_stack[0], 1, &stack_addr);

    // load code to frame 0
    char code[22][MAX_INSTRUCTION_CHAR] = {
        // set PID = 0;
        "mov    $0x0,%rbx",     // 0x00400000
        // fork
        "mov    $0x39, %rax",
        "int    $0x80",
        // check fork result to detect parent or child
        "mov    %rax,%rbx",
        "cmpq   $0x0,%rbx",
        // not returns 0, then parent process
        "jne    $0x00400380",
        // child LOOP: print child
        "movq   $0x0a646c696863, %rbx",   // 0x00400180
        "pushq  %rbx",
        "movq   $1, %rax",
        "movq   $1, %rdi",
        "movq   %rsp, %rsi",
        "movq   $6, %rdx",
        "int    $0x80",
        "jmp    $0x00400200",
        // LOOP: parent
        // parent LOOP: print parent
        "movq   $0x000a746e65726170, %rbx", // 0x00400380
        "pushq  %rbx",
        "movq   $1, %rax",
        "movq   $1, %rdi",
        "movq   %rsp, %rsi",
        "movq   $7, %rdx",
        "int    $0x80",
        "jmp    $0x00400400",
    };
    memcpy(
        (char *)(&pm[0]),
        &code, sizeof(char) * 22 * MAX_INSTRUCTION_CHAR);

    // create kernel stacks for trap into kernel
    kstack_t *stack_buf = aligned_alloc(KERNEL_STACK_SIZE, KERNEL_STACK_SIZE);
    uint64_t p1_stack_bottom = (uint64_t)stack_buf;
    p1.kstack = stack_buf;
    p1.kstack->threadinfo.pcb = &p1;

    // run p1
    tr_global_tss.ESP0 = p1_stack_bottom + KERNEL_STACK_SIZE;

    cpu_controls.cr3 = p1.mm.pgd_paddr;
    idt_init();
    syscall_init();

    // this should trigger page fault
    for (int i = 0; i < 100; ++i)
    {
        instruction_cycle();
    }

    printf("\033[32;1m\tPass\033[0m\n");
}

int main()
{
    TestFork();
    return 0;
}