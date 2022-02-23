/* BCST - Introduction to Computer Systems
 * Author:      yangminz@outlook.com
 * Github:      https://github.com/yangminz/bcst_csapp
 * Bilibili:    https://space.bilibili.com/4564101
 * Zhihu:       https://www.zhihu.com/people/zhao-yang-min
 * This project (code repository and videos) is exclusively owned by yangminz 
 * and shall not be used for commercial and profitting purpose 
 * without yangminz's permission.
 */

#include <stdint.h>
#include <stdlib.h>
#include <setjmp.h>

// include guards to prevent double declaration of any identifiers 
// such as types, enums and static variables
#ifndef INTERRUPT_GUARD
#define INTERRUPT_GUARD

// the struct of trap frame when interrupt on kernel stack
// executed by hardware CPU
typedef struct STRUCT_TRAPFRAME
{
    // error code
    uint64_t rip;
    // CS: counter segment
    // eflags
    uint64_t rsp;
    // SS: stack segment;
} trapframe_t;

// the struct of user frame when interrupt on kernel stack
// managed by software OS (in handler)
typedef struct STRUCT_USERFRAME
{
    cpu_reg_t regs;
    cpu_flags_t flags;
} userframe_t;

void idt_init();

void interrupt_stack_switching(uint64_t int_vec);
void interrupt_return_stack_switching();

uint64_t get_kstack_top_TSS();
uint64_t get_kstack_RSP();

/*  You will learn this from CSAPP: Exceptional Control Flow: Nonlocal Jumps.
 *  Nonlocal jump is a form of user-level exceptional control flow provided by C.
 *  We will use nonlocal jumps to implement interrut return.
 *  
 *  Think this: we implement 3 kinds of interrupts:
 *      -   System call
 *      -   Timer interrupt
 *      -   Page fault
 *  But their return addresses are different. And OS scheduling & context switch
 *  makes it even more complicated.
 * 
 *      System call
 *              process 1, int $0x80
 *                      Interrupt happens inside the `int` instruction.
 *                      So to successfully return to next process 1 instruction,
 *                      the return instruction to be pushed is the next.
 *                      THUS, WE NEED TO INCREMENT RIP BEFORE INVOKING INTERRUPT
 *              process 1, ret addr
 * 
 *                  instruction_cycle
 *                      int_handler
 *                          increase_pc // this must be called before interrupt
 *                          interrupt_stack_switching
 *                                      // so the pushed RIP will be the next instruction
 *                              syscall_handler
 *                                  os_schedule
 *                              interrupt_return_stack_switching 
 *                              // jump to the return instruction of process 2
 * 
 *      Timer interrupt
 *              process 1, add
 *                      Interrupt happens between the execution of these 2 instructions
 *                      So the execution of instruction i should be completed.
 *                      AND RIP PUSHED TO TRAP FRAME IS ALREADY RETURN ADDRESS
 *              process 1, ret addr
 * 
 *                  instruction_cycle
 *                      add_handler
 *                      interrupt_stack_switching
 *                                  // so the pushed RIP will be the next instruction
 *                          timer_handler
 *                              os_schedule
 *                          interrupt_return_stack_switching 
 *                          // jump to the return instruction of process 2
 * 
 *      Page fault
 *              process 1, mov $1234, [0x7fffffffffff]
 *                      Interrupt happens inside the `mov` instruction, the va2pa
 *                      address translation phase. Check this call stack
 *                  
 *                  instruction_cycle
 *                      mov_handler
 *                          va2pa
 *                              page_walk
 *                                  interrupt_stack_switching
 *                                          // Since RIP has not been updated, RIP pushed to 
 *                                          // trap frame is just this `mov` instruction.
 *                                          // Thus when OS sched back to process 1,
 *                                          // this instruction would be rescheduled.
 *                                          // And physical memory is not touched since 
 *                                          // address translation fails.
 *                                      pagefault_handler
 *                                          os_schedule
 *                                      interrupt_return_stack_switching 
 *                                      // jump to the return instruction of process 2
 *                          cpu_write64bits_dram    // will not be executed due to non-local jump
 *                          increase_pc             // will not be executed due to non-local jump
 */
jmp_buf USER_INSTRUCTION_ON_IRET;

#endif