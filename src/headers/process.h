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
    // TODO: add thread_info
} kstack_t;

void syscall_init();
void do_syscall(int syscall_no);

#endif