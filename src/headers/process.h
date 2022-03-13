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
#include "headers/memory.h"

// include guards to prevent double declaration of any identifiers 
// such as types, enums and static variables
#ifndef     PROCESS_GUARD
#define     PROCESS_GUARD

#define     KERNEL_STACK_SIZE   (8192)

typedef union KERNEL_STACK_STRUCT
{    
    uint8_t stack[KERNEL_STACK_SIZE];
    struct
    {
        struct PROCESS_CONTROL_BLOCK_STRUCT *pcb;
    } threadinfo;
} kstack_t;

typedef struct STRUCT_PROCESS_CONTEXT
{
    // attention: the registers & flags here are all in kernel mode
    cpu_reg_t regs;
    cpu_flags_t flags;
} context_t;

typedef struct PROCESS_CONTROL_BLOCK_STRUCT
{
    uint64_t pid;

    struct
    {
        // page global directory
        // This value is what's in CR3 register right now
        union
        {
            uint64_t pgd_paddr;
            pte123_t *pgd;
        };

        // TODO: vm area
    } mm;
    
    kstack_t *kstack;

    // it's easier to store the context to PCB
    context_t context;

    struct PROCESS_CONTROL_BLOCK_STRUCT *next;
    struct PROCESS_CONTROL_BLOCK_STRUCT *prev;
} pcb_t;

void syscall_init();

pcb_t *get_current_pcb();

#endif