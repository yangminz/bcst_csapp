/* BCST - Introduction to Computer Systems
 * Author:      yangminz@outlook.com
 * Github:      https://github.com/yangminz/bcst_csapp
 * Bilibili:    https://space.bilibili.com/4564101
 * Zhihu:       https://www.zhihu.com/people/zhao-yang-min
 * This project (code repository and videos) is exclusively owned by yangminz 
 * and shall not be used for commercial and profitting purpose 
 * without yangminz's permission.
 */

#include<stdio.h>
#include<string.h>
#include<headers/cpu.h>
#include<headers/memory.h>
#include<headers/common.h>

#define MAX_NUM_INSTRUCTION_CYCLE 100

static void TestAddFunctionCallAndComputation();
static void TestString2Uint();
static void TestSumRecursiveCondition();

// symbols from isa and sram
void print_register(core_t *cr);
void print_stack(core_t *cr);

void TestParsingOperand();
void TestParsingInstruction();

int main()
{
    TestAddFunctionCallAndComputation();
    TestSumRecursiveCondition();
    return 0;
}

static void TestString2Uint()
{
}

static void TestAddFunctionCallAndComputation()
{
    ACTIVE_CORE = 0x0;
    
    core_t *cr = (core_t *)&cores[ACTIVE_CORE];

    // init state
    cr->reg.rax = 0xabcd;
    cr->reg.rbx = 0x8000670;
    cr->reg.rcx = 0x8000670;
    cr->reg.rdx = 0x12340000;
    cr->reg.rsi = 0x7ffffffee208;
    cr->reg.rdi = 0x1;
    cr->reg.rbp = 0x7ffffffee110;
    cr->reg.rsp = 0x7ffffffee0f0;

    write64bits_dram(va2pa(0x7ffffffee110, cr), 0x0000000000000000, cr);    // rbp
    write64bits_dram(va2pa(0x7ffffffee108, cr), 0x0000000000000000, cr);
    write64bits_dram(va2pa(0x7ffffffee100, cr), 0x0000000012340000, cr);
    write64bits_dram(va2pa(0x7ffffffee0f8, cr), 0x000000000000abcd, cr);
    write64bits_dram(va2pa(0x7ffffffee0f0, cr), 0x0000000000000000, cr);    // rsp

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
        writeinst_dram(va2pa(i * 0x40 + 0x00400000, cr), assembly[i], cr);
    }
    cr->rip = MAX_INSTRUCTION_CHAR * sizeof(char) * 11 + 0x00400000;

    printf("begin\n");
    int time = 0;
    while (time < 15)
    {
        instruction_cycle(cr);
        print_register(cr);
        print_stack(cr);
        time ++;
    } 

    // gdb state ret from func
    int match = 1;
    match = match && cr->reg.rax == 0x1234abcd;
    match = match && cr->reg.rbx == 0x8000670;
    match = match && cr->reg.rcx == 0x8000670;
    match = match && cr->reg.rdx == 0xabcd;
    match = match && cr->reg.rsi == 0x12340000;
    match = match && cr->reg.rdi == 0xabcd;
    match = match && cr->reg.rbp == 0x7ffffffee110;
    match = match && cr->reg.rsp == 0x7ffffffee0f0;
    
    if (match)
    {
        printf("register match\n");
    }
    else
    {
        printf("register mismatch\n");
    }

    match = match && (read64bits_dram(va2pa(0x7ffffffee110, cr), cr) == 0x0000000000000000); // rbp
    match = match && (read64bits_dram(va2pa(0x7ffffffee108, cr), cr) == 0x000000001234abcd);
    match = match && (read64bits_dram(va2pa(0x7ffffffee100, cr), cr) == 0x0000000012340000);
    match = match && (read64bits_dram(va2pa(0x7ffffffee0f8, cr), cr) == 0x000000000000abcd);
    match = match && (read64bits_dram(va2pa(0x7ffffffee0f0, cr), cr) == 0x0000000000000000); // rsp

    if (match)
    {
        printf("memory match\n");
    }
    else
    {
        printf("memory mismatch\n");
    }
}

static void TestSumRecursiveCondition()
{
    ACTIVE_CORE = 0x0;
    core_t *cr = (core_t *)&cores[ACTIVE_CORE];

    // init state
    cr->reg.rax = 0x8000630;
    cr->reg.rbx = 0x0;
    cr->reg.rcx = 0x8000650;
    cr->reg.rdx = 0x7ffffffee328;
    cr->reg.rsi = 0x7ffffffee318;
    cr->reg.rdi = 0x1;
    cr->reg.rbp = 0x7ffffffee230;
    cr->reg.rsp = 0x7ffffffee220;

    cr->flags.__cpu_flag_value = 0;

    write64bits_dram(va2pa(0x7ffffffee230, cr), 0x0000000008000650, cr);    // rbp
    write64bits_dram(va2pa(0x7ffffffee228, cr), 0x0000000000000000, cr);
    write64bits_dram(va2pa(0x7ffffffee220, cr), 0x00007ffffffee310, cr);    // rsp

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
        writeinst_dram(va2pa(i * 0x40 + 0x00400000, cr), assembly[i], cr);
    }
    cr->rip = MAX_INSTRUCTION_CHAR * sizeof(char) * 16 + 0x00400000;

    printf("begin\n");
    int time = 0;
    while ((cr->rip <= 18 * 0x40 + 0x00400000) &&
           time < MAX_NUM_INSTRUCTION_CYCLE)
    {
        instruction_cycle(cr);
        print_register(cr);
        print_stack(cr);
        time ++;
    } 

    // gdb state ret from func
    int match = 1;
    match = match && cr->reg.rax == 0x6;
    match = match && cr->reg.rbx == 0x0;
    match = match && cr->reg.rcx == 0x8000650;
    match = match && cr->reg.rdx == 0x3;
    match = match && cr->reg.rsi == 0x7ffffffee318;
    match = match && cr->reg.rdi == 0x0;
    match = match && cr->reg.rbp == 0x7ffffffee230;
    match = match && cr->reg.rsp == 0x7ffffffee220;
    
    if (match)
    {
        printf("register match\n");
    }
    else
    {
        printf("register mismatch\n");
    }

    match = match && (read64bits_dram(va2pa(0x7ffffffee230, cr), cr) == 0x0000000008000650); // rbp
    match = match && (read64bits_dram(va2pa(0x7ffffffee228, cr), cr) == 0x0000000000000006);
    match = match && (read64bits_dram(va2pa(0x7ffffffee220, cr), cr) == 0x00007ffffffee310); // rsp

    if (match)
    {
        printf("memory match\n");
    }
    else
    {
        printf("memory mismatch\n");
    }
}