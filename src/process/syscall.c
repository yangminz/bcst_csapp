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

typedef void (*syscall_handler_t)();

// the entry of syscall table
typedef struct SYSCALL_ENTRY_STRUCT
{
    syscall_handler_t handler;
} syscall_entry_t;

// table of syscalls
syscall_entry_t syscall_table[64];

// handlers of syscalls
static void write_handler();
static void getpid_handler();
static void fork_handler();
static void execve_handler();
static void exit_handler();
static void wait_handler();
static void kill_handler();

static void destory_user_registers()
{
    // when run kernel thread, user registers should be useless
    memset(&cpu_reg, 0, sizeof(cpu_reg));
    memset(&cpu_flags, 0, sizeof(cpu_flags));
}

// initialize of IDT
void syscall_init()
{
    syscall_table[01].handler = write_handler;
    syscall_table[39].handler = getpid_handler;
    syscall_table[57].handler = fork_handler;
    syscall_table[59].handler = execve_handler;
    syscall_table[60].handler = exit_handler;
    syscall_table[61].handler = wait_handler;
    syscall_table[62].handler = kill_handler;
}

static void write_handler()
{
    // assembly begin
    // get user thread parameters
    uint64_t file_no = cpu_reg.rdi;
    uint64_t buf_vaddr = cpu_reg.rsi;
    uint64_t buf_length = cpu_reg.rdx;
    // assembly end

    destory_user_registers();

    // The following resource are allocated on KERNEL STACK
    // TODO: this works only with NAVIE VA2PA
    for (int i = 0; i < buf_length; ++ i)
    {
        // print as yellow
        printf("\033[33;1m%c\033[0m", pm[va2pa(buf_vaddr + i)]);
    }
}

static void getpid_handler()
{}

static void fork_handler()
{
    uint64_t kernel_rsp = cpu_reg.rsp;
    destory_user_registers();
    cpu_reg.rsp = kernel_rsp;

    syscall_fork();
}

static void execve_handler()
{}

static void exit_handler()
{
    // assembly begin
    uint64_t exit_status = cpu_reg.rdi;
    // assembly end

    // The following resource are allocated on KERNEL STACK
    printf("\033[33;1mGood Bye ~~~\n\033[0m");
}

static void wait_handler()
{}

static void kill_handler()
{}

void do_syscall(int syscall_no)
{
    assert(0 <= syscall_no && syscall_no <= 63);
    syscall_table[syscall_no].handler();
}