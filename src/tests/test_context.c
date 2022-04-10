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

static void load_code_physically(int pid, address_t *code_addr)
{
    /* this is a while loop like:
     * while(1) { printf("p%d\n", pid); }
     */

    // code to be copied to physical page
    char code[8][MAX_INSTRUCTION_CHAR] = {
        // open stack for string buffer
        // "p?\n"
        "movq $0x000a3170, %rbx",   // 0: 0x00400000
        "pushq %rbx",               // 1: 0x00400040
        "movq $1, %rax",            // 2
        "movq $1, %rdi",            // 3
        "movq %rsp, %rsi",          // 4: 0x00400100
        "movq $13, %rdx",           // 5
        "int $0x80",                // 6: 0x00400180
        "jmp 0x00400100"            // 7: jump to 4
    };
    // the correct execution is:
    // 000, 040, 080, 0c0, [100, 140, 180, 1c0], [100, 140, 180, 1c0], [100, 140, 180, 1c0], ...
    code[0][13] = (uint8_t)pid + '0';
    uint8_t *start = &pm[(pid - 1) * PAGE_SIZE + code_addr->vpo];
    memcpy((char *)start, &code, sizeof(char) * 8 * MAX_INSTRUCTION_CHAR);
}

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

static void TestContextSwitching()
{
    printf("Testing context switching ...\n");

    // init state
    cpu_reg.rsp = 0x7ffffffee0f0;
    cpu_pc.rip = 0x00400000;

    address_t stack_addr = {.address_value = cpu_reg.rsp};
    address_t code_addr = {.address_value = cpu_pc.rip};

    // prepare 3 processes as circular doubly linked list
    pcb_t p1, p2, p3;
    p1.next = &p2;
    p2.next = &p3;
    p3.next = &p1;
    p1.prev = &p3;
    p2.prev = &p1;
    p3.prev = &p2;

    p1.pid = 1;
    p2.pid = 2;
    p3.pid = 3;

    // prepare the page tables for these processes
    // each process will use 2 page tables: 
    // one for user stack, one for user data & code
    pte123_t p1_pgd[512];
    pte123_t p2_pgd[512];
    pte123_t p3_pgd[512];
    p1.mm.pgd = &p1_pgd[0];
    p2.mm.pgd = &p2_pgd[0];
    p3.mm.pgd = &p3_pgd[0];

    page_map_init();

    // please think why we do not need to map the stack page directly?
    // how will page fault handling help us with this?

    // p1's code page
    pte123_t p1_pud[512];
    pte123_t p1_pmd[512];
    pte4_t p1_pt_code[512];
    link_page_table(&p1_pgd[0], &p1_pud[0], &p1_pmd[0], &p1_pt_code[0], 0, &code_addr);
    load_code_physically(1, &code_addr);

    // p2's code page
    pte123_t p2_pud[512];
    pte123_t p2_pmd[512];
    pte4_t p2_pt_code[512];
    link_page_table(&p2_pgd[0], &p2_pud[0], &p2_pmd[0], &p2_pt_code[0], 1, &code_addr);
    load_code_physically(2, &code_addr);

    // p3's code page
    pte123_t p3_pud[512];
    pte123_t p3_pmd[512];
    pte4_t p3_pt_code[512];
    link_page_table(&p3_pgd[0], &p3_pud[0], &p3_pmd[0], &p3_pt_code[0], 2, &code_addr);
    load_code_physically(3, &code_addr);

    // create kernel stacks
    uint8_t stack_buf[8192 * 4];

    uint64_t p1_stack_bottom = (((uint64_t)&stack_buf[8192]) >> 13) << 13;
    uint64_t p2_stack_bottom = p1_stack_bottom + KERNEL_STACK_SIZE;
    uint64_t p3_stack_bottom = p2_stack_bottom + KERNEL_STACK_SIZE;

    p1.kstack = (kstack_t *)p1_stack_bottom;
    p2.kstack = (kstack_t *)p2_stack_bottom;
    p3.kstack = (kstack_t *)p3_stack_bottom;

    p1.kstack->threadinfo.pcb = &p1;
    p2.kstack->threadinfo.pcb = &p2;
    p3.kstack->threadinfo.pcb = &p3;

    // create trap frames for p2, p3
    trapframe_t tf = {
        .rip = code_addr.vaddr_value,
        .rsp = stack_addr.vaddr_value
    };
    memcpy(
        (trapframe_t *)(p2_stack_bottom + KERNEL_STACK_SIZE - sizeof(trapframe_t)),
        &tf, sizeof(trapframe_t));
    memcpy(
        (trapframe_t *)(p3_stack_bottom + KERNEL_STACK_SIZE - sizeof(trapframe_t)),
        &tf, sizeof(trapframe_t));

    // create user frames for p2, p3
    // create contexts for p2, p3
    // actually user frames & contexts does not matter at the starting point
    p2.context.regs.rsp = p2_stack_bottom + KERNEL_STACK_SIZE - sizeof(trapframe_t) - sizeof(userframe_t);
    p3.context.regs.rsp = p3_stack_bottom + KERNEL_STACK_SIZE - sizeof(trapframe_t) - sizeof(userframe_t);

    // run p1
    tr_global_tss.ESP0 = p1_stack_bottom + KERNEL_STACK_SIZE;

    cpu_controls.cr3 = p1.mm.pgd_paddr;

    idt_init();
    syscall_init();
    
    printf("begin\n");
    int time = 0;
    while (time < 100)
    {
        instruction_cycle();
#ifdef DEBUG_INSTRUCTION_CYCLE_INFO_REG_STACK
        print_register();
        print_stack();
#endif
        time ++;
    }

    printf("\033[32;1m\tPass\033[0m\n");
}

int main()
{    
    TestContextSwitching();
    return 0;
}