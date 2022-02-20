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

typedef struct PROCESS_CONTROL_BLOCK_STRUCT
{
    uint64_t pid;

    struct
    {
        // page global directory
        // This value is what's in CR3 register right now
        uint64_t pgd_paddr;

        // TODO: vm area
    } mm;
    
    kstack_t *kstack;
} pcb_t;

void syscall_init();
void do_syscall(int syscall_no);

void os_schedule();

#endif