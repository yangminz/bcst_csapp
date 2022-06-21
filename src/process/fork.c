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

static uint64_t fork_naive_copy();
static uint64_t fork_cow();

uint64_t syscall_fork()
{
    return fork_naive_copy();
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
{}

static uint64_t fork_naive_copy()
{
    // Note that all system calls are actually compiled to binaries
    // Operating system itself is a binary, a software, executed by CPU
    // Here we use C language to write the fork
    // But actually, we should write instructions:
    // mov_handler
    // add_handler, etc.
    //
    // Now, let's start to write `fork`
    
    // DIRECT COPY
    // find all pages of current process:
    pcb_t *parent_pcb = get_current_pcb();

    // ATTENTION HERE!!!
    // In a realistic OS, you need to allocate kernel pages as well
    // And then copy the data on parent kernel page to child's.
    pcb_t *child_pcb = KERNEL_malloc(sizeof(pcb_t));

    memcpy(child_pcb, parent_pcb, sizeof(pcb_t));

    // update child PID
    child_pcb->pid = get_newpid();

    // TODO: copy the entire page table of parent

    // TODO: find physical frames to copy the pages of parent

    // All copy works are done here

    // Fork's secret:
    // call once, return twice
    update_userframe_returnvalue(parent_pcb, child_pcb->pid);
    update_userframe_returnvalue(child_pcb, 0);

    // TODO: add child process PCB to linked list for scheduling
    // it's better the child process is right after parent
    return 0;
}

static uint64_t fork_cow()
{
    return 0;
}