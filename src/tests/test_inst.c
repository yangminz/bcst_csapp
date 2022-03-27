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

void mov_handler             (od_t *src_od, od_t *dst_od) {};
void push_handler            (od_t *src_od, od_t *dst_od) {};
void pop_handler             (od_t *src_od, od_t *dst_od) {};
void leave_handler           (od_t *src_od, od_t *dst_od) {};
void call_handler            (od_t *src_od, od_t *dst_od) {};
void ret_handler             (od_t *src_od, od_t *dst_od) {};
void add_handler             (od_t *src_od, od_t *dst_od) {};
void sub_handler             (od_t *src_od, od_t *dst_od) {};
void cmp_handler             (od_t *src_od, od_t *dst_od) {};
void jne_handler             (od_t *src_od, od_t *dst_od) {};
void jmp_handler             (od_t *src_od, od_t *dst_od) {};
void lea_handler             (od_t *src_od, od_t *dst_od) {};

void parse_instruction(const char *str, inst_t *inst);
void parse_operand(const char *str, od_t *od);
uint64_t compute_operand(od_t *od);

static int operand_equal(od_t *a, od_t *b)
{
    if (a == NULL && b == NULL)
    {
        return 1;
    }

    if (a == NULL || b == NULL)
    {
        return 0;
    }

    int equal = 1;
    equal = equal && (a->type == b->type);
    equal = equal && (a->value == b->value);

    return equal;
}

static int instruction_equal(inst_t *a, inst_t *b)
{
    if (a == NULL && b == NULL)
    {
        return 1;
    }

    if (a == NULL || b == NULL)
    {
        return 0;
    }

    int equal = 1;
    equal = equal && (a->op == b->op);
    equal = equal && operand_equal(&a->src, &b->src);
    equal = equal && operand_equal(&a->dst, &b->dst);

    return equal;
}

static void TestParsingInstruction()
{
    printf("Testing instruction parsing ...\n");

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
        "callq  0",                 // 13
        "mov    %rax,-0x8(%rbp)",   // 14
    };

    inst_t std_inst[15] = {
        // push   %rbp
        {
            .op = &push_handler,
            .src = 
                {
                    .type = OD_REG,
                    .value = (uint64_t)(&cpu_reg.rbp),
                },
            .dst = 
                {
                    .type = OD_EMPTY,
                    .value = 0,
                }
        },        
        // mov    %rsp,%rbp
        {
            .op = &mov_handler, 
            .src = {
                .type = OD_REG, 
                .value = (uint64_t)(&cpu_reg.rsp), 
            }, 
            .dst = {
                .type = OD_REG,
                .value = (uint64_t)(&cpu_reg.rbp), 
            }
        },
        // mov    %rdi,-0x18(%rbp)
        {
            .op = &mov_handler, 
            .src = {
                .type = OD_REG,
                .value = (uint64_t)(&cpu_reg.rdi), 
            }, 
            .dst = {
                .type = OD_MEM,
                .value = cpu_reg.rbp - 0x18,
            }
        },
        // mov    %rsi,-0x20(%rbp)
        {
            .op = &mov_handler,
            .src = {
                .type = OD_REG, 
                .value = (uint64_t)(&cpu_reg.rsi),
            }, 
            .dst = {
                .type = OD_MEM,
                .value = cpu_reg.rbp - 0x20,
            }
        },
        // mov    -0x18(%rbp),%rdx
        {
            .op = &mov_handler, 
            .src = {
                .type = OD_MEM,
                .value = cpu_reg.rbp - 0x18,
            },
            .dst = {
                .type = OD_REG,
                .value = (uint64_t)(&cpu_reg.rdx),
            }
        },
        // mov    -0x20(%rbp),%rax
        {
            .op = &mov_handler,
            .src = {
                .type = OD_MEM,
                .value = cpu_reg.rbp - 0x20,
            },
            .dst = {
                .type = OD_REG,
                .value = (uint64_t)(&cpu_reg.rax),
            }
        },
        // add    %rdx,%rax
        {
            .op = &add_handler,
            .src = {
                .type = OD_REG,
                .value = (uint64_t)(&cpu_reg.rdx),
            },
            .dst = {
                .type = OD_REG,
                .value = (uint64_t)(&cpu_reg.rax),
            }
        },
        // mov    %rax,-0x8(%rbp)
        {
            .op = &mov_handler,
            .src = {
                .type = OD_REG,
                .value = (uint64_t)(&cpu_reg.rax),
            },
            .dst = {
                .type = OD_MEM,
                .value = cpu_reg.rbp - 0x8,
            }
        },
        // mov    -0x8(%rbp),%rax
        {
            .op = &mov_handler,
            .src = {
                .type = OD_MEM,
                .value = cpu_reg.rbp - 0x8,
            },
            .dst = {
                .type = OD_REG,
                .value = (uint64_t)(&cpu_reg.rax),
            }
        },
        // pop    %rbp
        {
            .op = &pop_handler,
            .src = {
                .type = OD_REG,
                .value = (uint64_t)(&cpu_reg.rbp),
            },
            .dst = {
                .type = OD_EMPTY,
                .value = 0,
            }
        },
        // retq
        {
            .op = &ret_handler,
            .src = {
                .type = OD_EMPTY,
                .value = 0,
            },
            .dst = {
                .type = OD_EMPTY,
                .value = 0,
            }
        },
        // mov    %rdx,%rsi
        {
            .op = &mov_handler,
            .src = {
                .type = OD_REG,
                .value = (uint64_t)(&cpu_reg.rdx),
            },
            .dst = {
                .type = OD_REG,
                .value = (uint64_t)(&cpu_reg.rsi),
            }
        },
        // mov    %rax,%rdi
        {
            .op = &mov_handler,
            .src = {
                .type = OD_REG,
                .value = (uint64_t)(&cpu_reg.rax),
            },
            .dst = {
                .type = OD_REG,
                .value = (uint64_t)(&cpu_reg.rdi),
            }
        },
        // callq  0
        {
            .op = &call_handler,
            .src = {
                .type = OD_MEM,
                .value = 0,
                .value = 0
            },
            .dst = {
                .type = OD_EMPTY,
                .value = 0,
            }
        },
        // mov    %rax,-0x8(%rbp)
        {
            .op = &mov_handler,
            .src = {
                .type = OD_REG,
                .value = (uint64_t)(&cpu_reg.rax),
            },
            .dst = {
                .type = OD_MEM,
                .value = cpu_reg.rbp - 0x8,
            }
        },
    };
    
    inst_t inst_parsed;

    for (int i = 0; i < 15; ++ i)
    {
        parse_instruction(assembly[i], &inst_parsed);
        assert(instruction_equal(&std_inst[i], &inst_parsed) == 1);
    }

    printf("\033[32;1m\tPass\033[0m\n");
}

int main()
{
    TestParsingInstruction();
    return 0;
}