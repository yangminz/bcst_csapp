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
#include "headers/interrupt.h"

typedef void (*interrupt_handler_t)();

// the entry of IDT/IVT
typedef struct IDT_ENTRY_STRUCT
{
    interrupt_handler_t handler;
} idt_entry_t;

// Interrupt Descriptor/Vector Table
idt_entry_t idt[256];

// handlers of IDT
void protectionfault_handler();     // trap gate - exception
void pagefault_handler();           // trap gate - exception
void syscall_handler();             // trap gate - software interrupt
void timer_handler();               // interrupt gate - local APIC

// initialize of IDT
void idt_init()
{
    idt[13].handler = protectionfault_handler;
    idt[14].handler = pagefault_handler;
    idt[0x80].handler = syscall_handler;
}


// call interrupt with stack switching (user --> kernel)
void call_interrupt_stack_switching(uint64_t int_vec)
{
    assert(0 <= int_vec && int_vec <= 255);

    //  1.  Temporarily saves (internally) the current contents of 
    //      the SS, ESP, EFLAGS, CS, and EIP registers.
    //  TODO: SS & CS
    uint64_t rsp_user = cpu_reg.rsp;
    uint64_t rip_user = cpu_pc.rip;

    //  2.  Loads the segment selector and stack pointer for the new stack 
    //      (that is, the stack for the privilege level being called) 
    //      from the TSS into the SS and ESP registers and switches to the new stack.
    //  TRICK: we do not use GDT. Instead, TR directly points to TSS (actually Stack0)
    tss_s0_t *tss_s0 = (tss_s0_t *)cpu_task_register;
    uint64_t kstack = tss_s0->ESP0; // should be 8KB aligned
    // stack switching
    // this kstack is allocated in the heap of emulator
    cpu_reg.rsp = kstack;

    //  3.  Pushes the temporarily saved SS, ESP, EFLAGS, CS, and EIP values 
    //      for the interrupted procedure’s stack onto the new stack.
    //  TODO: SS & CS
    //  TODO: kernel address space & page table mapping
    //  TODO: use malloc for kernel space
    cpu_reg.rsp = cpu_reg.rsp - 8;
    *(uint64_t *)cpu_reg.rsp = rsp_user;
    cpu_reg.rsp = cpu_reg.rsp - 8;
    *(uint64_t *)cpu_reg.rsp = rip_user;
    /*  If we implement the kernel address space on physical memory in memory.h ...
        cpu_write64bits_dram(
            va2pa(cpu_reg.rsp),
            rsp_user);
        cpu_write64bits_dram(
            va2pa(cpu_reg.rsp), 
            rip_user);
    */

    //  4.  Pushes an error code on the new stack (if appropriate).

    //  5.  Loads the segment selector for the new code segment and 
    //      the new instruction pointer (from the interrupt gate or trap gate) 
    //      into the CS and EIP registers, respectively.
    interrupt_handler_t handler = idt[int_vec].handler;

    //  6.  If the call is through an interrupt gate, 
    //      clears the IF flag in the EFLAGS register.
    
    //  Not needed because our interrupt is in isa.c:instruction_cycle
    //  but for kernel routine the code is running in host's CPU
    //  so emulator's interrupt cannot interrupt host's CPU

    //  7.  Begins execution of the handler procedure at the new privilege level.
    handler();
}


// interrupt handlers

void syscall_handler()
{
    uint64_t syscall_num = cpu_reg.rax;
    return;
}