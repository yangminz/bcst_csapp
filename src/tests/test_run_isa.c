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
#include "headers/instruction.h"
#include "headers/interrupt.h"
#include "headers/process.h"

static void print_register()
{
    printf("rax = %16lx\trbx = %16lx\trcx = %16lx\trdx = %16lx\n",
        cpu_reg.rax, cpu_reg.rbx, cpu_reg.rcx, cpu_reg.rdx);
    printf("rsi = %16lx\trdi = %16lx\trbp = %16lx\trsp = %16lx\n",
        cpu_reg.rsi, cpu_reg.rdi, cpu_reg.rbp, cpu_reg.rsp);
    printf("rip = %16lx\n", cpu_pc.rip);
    printf("CF = %u\tZF = %u\tSF = %u\tOF = %u\n",
        cpu_flags.CF, cpu_flags.ZF, cpu_flags.SF, cpu_flags.OF);
}

static void print_stack()
{
    int n = 10;    
    uint64_t *high = (uint64_t*)&pm[va2pa(cpu_reg.rsp)];
    high = &high[n];
    uint64_t va = cpu_reg.rsp + n * 8;

    for (int i = 0; i < 2 * n; ++ i)
    {
        uint64_t *ptr = (uint64_t *)(high - i);
        printf("0x%16lx : %16lx", va, (uint64_t)*ptr);

        if (i == n)
        {
            printf(" <== rsp");
        }
        printf("\n");
        va -= 8;
    }
}

static void TestSyscallPrintHelloWorld()
{
    printf("Testing syscall to print 'hello world\n' ...\n");
 
    // init state
    cpu_reg.rsp = 0x7ffffffee0f0;

    char assembly[12][MAX_INSTRUCTION_CHAR] = {
        // open stack for string buffer
        // "rld\n"
        "movq $0x000a646c72, %rbx",
        "pushq %rbx",
        // "hello wo"
        "movq $0x6f77206f6c6c6568, %rbx",
        "pushq %rbx",
        // call write for the string to stdout
        "movq $1, %rax",
        "movq $1, %rdi",
        "movq %rsp, %rsi",
        "movq $13, %rdx",
        "int $0x80",
        // call exit
        "movq $60, %rax",
        "movq $0, %rdi",
        "int $0x80",
    };

    // copy to physical memory
    for (int i = 0; i < 12; ++ i)
    {
        cpu_writeinst_dram(va2pa(i * 0x40 + 0x00400000), assembly[i]);
    }
    cpu_pc.rip = 0x00400000;

    // prepare a kernel stack
    uint8_t kstack_buf[8192 * 2];
    uint64_t k_temp = (uint64_t)&kstack_buf[8192];

    kstack_t *kstack = (kstack_t *)((k_temp >> 13) << 13);
    tss_s0_t tss;

    tss.ESP0 = (uint64_t)kstack + 8192;
    cpu_task_register = (uint64_t)&tss;

    pcb_t curr;
    curr.prev = &curr;
    curr.next = &curr;
    curr.kstack = kstack;
    kstack->threadinfo.pcb = &curr;

    idt_init();
    syscall_init();
    
    printf("begin\n");
    int time = 0;
    while (time < 12)
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

static void TestAddFunctionCallAndComputation()
{
    printf("Testing add function call ...\n");
 
    // init state
    cpu_reg.rax = 0xabcd;
    cpu_reg.rbx = 0x8000670;
    cpu_reg.rcx = 0x8000670;
    cpu_reg.rdx = 0x12340000;
    cpu_reg.rsi = 0x7ffffffee208;
    cpu_reg.rdi = 0x1;
    cpu_reg.rbp = 0x7ffffffee110;
    cpu_reg.rsp = 0x7ffffffee0f0;

    cpu_write64bits_dram(va2pa(0x7ffffffee110), 0x0000000000000000);    // rbp
    cpu_write64bits_dram(va2pa(0x7ffffffee108), 0x0000000000000000);
    cpu_write64bits_dram(va2pa(0x7ffffffee100), 0x0000000012340000);
    cpu_write64bits_dram(va2pa(0x7ffffffee0f8), 0x000000000000abcd);
    cpu_write64bits_dram(va2pa(0x7ffffffee0f0), 0x0000000000000000);    // rsp

    // 2 before call
    // 3 after call before push
    // 5 after rbp
    // 13 before pop
    // 14 after pop before ret
    // 15 after ret
    char assembly[15][MAX_INSTRUCTION_CHAR] = {
        "push   %rbp",              // 0
        "mov    %rsp,%rbp",         // 1
        "mov    %rdi,-0x18(%rbp)",  // 2
        "mov    %rsi,-0x20(%rbp)",  // 3
        "mov    -0x18(%rbp),%rdx",  // 4
        "mov    -0x20(%rbp),%rax",  // 5
        "add    %rdx,%rax",         // 6
        "mov    %rax,-0x8(%rbp)",   // 7
        "mov    -0x8(%rbp),%rax",   // 8
        "pop    %rbp",              // 9
        "retq",                     // 10
        "mov    %rdx,%rsi",         // 11
        "mov    %rax,%rdi",         // 12
        "callq  0x00400000",        // 13
        "mov    %rax,-0x8(%rbp)",   // 14
    };

    // copy to physical memory
    for (int i = 0; i < 15; ++ i)
    {
        cpu_writeinst_dram(va2pa(i * 0x40 + 0x00400000), assembly[i]);
    }
    cpu_pc.rip = MAX_INSTRUCTION_CHAR * sizeof(char) * 11 + 0x00400000;

    printf("begin\n");
    int time = 0;
    while (time < 15)
    {
        instruction_cycle();
#ifdef DEBUG_INSTRUCTION_CYCLE_INFO_REG_STACK
        print_register();
        print_stack();
#endif
        time ++;
    } 

    // gdb state ret from func
    assert(cpu_reg.rax == 0x1234abcd);
    assert(cpu_reg.rbx == 0x8000670);
    assert(cpu_reg.rcx == 0x8000670);
    assert(cpu_reg.rdx == 0xabcd);
    assert(cpu_reg.rsi == 0x12340000);
    assert(cpu_reg.rdi == 0xabcd);
    assert(cpu_reg.rbp == 0x7ffffffee110);
    assert(cpu_reg.rsp == 0x7ffffffee0f0);

    assert(cpu_read64bits_dram(va2pa(0x7ffffffee110)) == 0x0000000000000000); // rbp
    assert(cpu_read64bits_dram(va2pa(0x7ffffffee108)) == 0x000000001234abcd);
    assert(cpu_read64bits_dram(va2pa(0x7ffffffee100)) == 0x0000000012340000);
    assert(cpu_read64bits_dram(va2pa(0x7ffffffee0f8)) == 0x000000000000abcd);
    assert(cpu_read64bits_dram(va2pa(0x7ffffffee0f0)) == 0x0000000000000000); // rsp

    printf("\033[32;1m\tPass\033[0m\n");
}

static void TestSumRecursiveCondition()
{
    printf("Testing sum recursive function call ...\n");

    // init state
    cpu_reg.rax = 0x8000630;
    cpu_reg.rbx = 0x0;
    cpu_reg.rcx = 0x8000650;
    cpu_reg.rdx = 0x7ffffffee328;
    cpu_reg.rsi = 0x7ffffffee318;
    cpu_reg.rdi = 0x1;
    cpu_reg.rbp = 0x7ffffffee230;
    cpu_reg.rsp = 0x7ffffffee220;

    cpu_flags.__flags_value = 0;

    cpu_write64bits_dram(va2pa(0x7ffffffee230), 0x0000000008000650);    // rbp
    cpu_write64bits_dram(va2pa(0x7ffffffee228), 0x0000000000000000);
    cpu_write64bits_dram(va2pa(0x7ffffffee220), 0x00007ffffffee310);    // rsp

    char assembly[19][MAX_INSTRUCTION_CHAR] = {
        "push   %rbp",              // 0
        "mov    %rsp,%rbp",         // 1
        "sub    $0x10,%rsp",        // 2
        "mov    %rdi,-0x8(%rbp)",   // 3
        "cmpq   $0x0,-0x8(%rbp)",   // 4
        "jne    0x400200",          // 5: jump to 8
        "mov    $0x0,%eax",         // 6
        "jmp    0x400380",          // 7: jump to 14
        "mov    -0x8(%rbp),%rax",   // 8
        "sub    $0x1,%rax",         // 9
        "mov    %rax,%rdi",         // 10
        "callq  0x00400000",        // 11
        "mov    -0x8(%rbp),%rdx",   // 12
        "add    %rdx,%rax",         // 13
        "leaveq ",                  // 14
        "retq   ",                  // 15
        "mov    $0x3,%edi",         // 16
        "callq  0x00400000",        // 17
        "mov    %rax,-0x8(%rbp)",   // 18
    };

    // copy to physical memory
    for (int i = 0; i < 19; ++ i)
    {
        cpu_writeinst_dram(va2pa(i * 0x40 + 0x00400000), assembly[i]);
    }
    cpu_pc.rip = MAX_INSTRUCTION_CHAR * sizeof(char) * 16 + 0x00400000;

    printf("begin\n");
    int time = 0;
    while ((cpu_pc.rip <= 18 * 0x40 + 0x00400000) &&
           time < MAX_NUM_INSTRUCTION_CYCLE)
    {
        instruction_cycle();
#ifdef DEBUG_INSTRUCTION_CYCLE_INFO_REG_STACK
        print_register();
        print_stack();
#endif
        time ++;
    } 

    // gdb state ret from func
    assert(cpu_reg.rax == 0x6);
    assert(cpu_reg.rbx == 0x0);
    assert(cpu_reg.rcx == 0x8000650);
    assert(cpu_reg.rdx == 0x3);
    assert(cpu_reg.rsi == 0x7ffffffee318);
    assert(cpu_reg.rdi == 0x0);
    assert(cpu_reg.rbp == 0x7ffffffee230);
    assert(cpu_reg.rsp == 0x7ffffffee220);
    assert(cpu_read64bits_dram(va2pa(0x7ffffffee230)) == 0x0000000008000650); // rbp
    assert(cpu_read64bits_dram(va2pa(0x7ffffffee228)) == 0x0000000000000006);
    assert(cpu_read64bits_dram(va2pa(0x7ffffffee220)) == 0x00007ffffffee310); // rsp

    printf("\033[32;1m\tPass\033[0m\n");
}

int main()
{
    //TestAddFunctionCallAndComputation();
    //TestSumRecursiveCondition();

    TestSyscallPrintHelloWorld();

    finally_cleanup();
    return 0;
}