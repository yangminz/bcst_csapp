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
#include "headers/algorithm.h"

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

typedef struct VIRTUAL_MEMORY_AREA_STRUCT
{
    struct VIRTUAL_MEMORY_AREA_STRUCT *prev;
    struct VIRTUAL_MEMORY_AREA_STRUCT *next;
    uint64_t vma_start;
    uint64_t vma_end;
    union
    {
        uint64_t mode_value;
        struct
        {
            uint64_t read:      1;
            uint64_t write:     1;
            uint64_t execute:   1;
            uint64_t private:   1;
        } vma_mode;
    };

    // actually we need inode here but we do not implement file system
    char filepath[128];

    // used for RBT indexing
    int rbt_color;
} vm_area_t;

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

        // virtual memory area
        linkedlist_internal_t vma;
    } mm;
    
    kstack_t *kstack;

    // it's easier to store the context to PCB
    context_t context;

    struct PROCESS_CONTROL_BLOCK_STRUCT *next;
    struct PROCESS_CONTROL_BLOCK_STRUCT *prev;
} pcb_t;

void syscall_init();

pcb_t *get_current_pcb();

void setup_pagetable_from_vma(pcb_t *proc);
int vma_add_area(pcb_t *proc, vm_area_t *area);

#endif