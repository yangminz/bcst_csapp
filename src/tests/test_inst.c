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
    equal = equal && (a->imm == b->imm);
    equal = equal && (a->scal == b->scal);
    equal = equal && (a->reg1 == b->reg1);
    equal = equal && (a->reg2 == b->reg2);

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
            .op = INST_PUSH,
            .src = 
                {
                    .type = OD_REG,
                    .imm = 0,
                    .scal = 0,
                    .reg1 = (uint64_t)(&cpu_reg.rbp),
                    .reg2 = 0
                },
            .dst = 
                {
                    .type = OD_EMPTY,
                    .imm = 0,
                    .scal = 0,
                    .reg1 = 0,
                    .reg2 = 0
                }
        },        
        // mov    %rsp,%rbp
        {
            .op = INST_MOV, 
            .src = {
                .type = OD_REG, 
                .imm = 0, 
                .scal = 0, 
                .reg1 = (uint64_t)(&cpu_reg.rsp), 
                .reg2 = 0
            }, 
            .dst = {
                .type = OD_REG,
                .imm = 0, 
                .scal = 0, 
                .reg1 = (uint64_t)(&cpu_reg.rbp), 
                .reg2 = 0}
        },
        // mov    %rdi,-0x18(%rbp)
        {
            .op = INST_MOV, 
            .src = {
                .type = OD_REG, 
                .imm = 0, 
                .scal = 0, 
                .reg1 = (uint64_t)(&cpu_reg.rdi), 
                .reg2 = 0
            }, 
            .dst = {
                .type = OD_MEM_IMM_REG1, 
                .imm = 0x1LL + (~0x18LL), 
                .scal = 0, 
                .reg1 = (uint64_t)(&cpu_reg.rbp), 
                .reg2 = 0
            }
        },
        // mov    %rsi,-0x20(%rbp)
        {
            .op = INST_MOV, 
            .src = {
                .type = OD_REG, 
                .imm = 0,
                .scal = 0,
                .reg1 = (uint64_t)(&cpu_reg.rsi),
                .reg2 = 0
            }, 
            .dst = {
                .type = OD_MEM_IMM_REG1,
                .imm = 0x1LL + (~0x20LL),
                .scal = 0,
                .reg1 = (uint64_t)(&cpu_reg.rbp),
                .reg2 = 0
            }
        },
        // mov    -0x18(%rbp),%rdx
        {
            .op = INST_MOV, 
            .src = {
                .type = OD_MEM_IMM_REG1,
                .imm = 0x1LL + (~0x18LL),
                .scal = 0,
                .reg1 = (uint64_t)(&cpu_reg.rbp),
                .reg2 = 0
            },
            .dst = {
                .type = OD_REG,
                .imm = 0,
                .scal = 0,
                .reg1 = (uint64_t)(&cpu_reg.rdx),
                .reg2 = 0
            }
        },
        // mov    -0x20(%rbp),%rax
        {
            .op = INST_MOV,
            .src = {
                .type = OD_MEM_IMM_REG1,
                .imm = 0x1LL + (~0x20LL),
                .scal = 0,
                .reg1 = (uint64_t)(&cpu_reg.rbp),
                .reg2 = 0
            },
            .dst = {
                .type = OD_REG,
                .imm = 0,
                .scal = 0,
                .reg1 = (uint64_t)(&cpu_reg.rax),
                .reg2 = 0
            }
        },
        // add    %rdx,%rax
        {
            .op = INST_ADD,
            .src = {
                .type = OD_REG,
                .imm = 0,
                .scal = 0,
                .reg1 = (uint64_t)(&cpu_reg.rdx),
                .reg2 = 0
            },
            .dst = {
                .type = OD_REG,
                .imm = 0,
                .scal = 0,
                .reg1 = (uint64_t)(&cpu_reg.rax),
                .reg2 = 0
            }
        },
        // mov    %rax,-0x8(%rbp)
        {
            .op = INST_MOV,
            .src = {
                .type = OD_REG,
                .imm = 0,
                .scal = 0,
                .reg1 = (uint64_t)(&cpu_reg.rax),
                .reg2 = 0
            },
            .dst = {
                .type = OD_MEM_IMM_REG1,
                .imm = 0x1LL + (~0x8LL),
                .scal = 0,
                .reg1 = (uint64_t)(&cpu_reg.rbp),
                .reg2 = 0
            }
        },
        // mov    -0x8(%rbp),%rax
        {
            .op = INST_MOV,
            .src = {
                .type = OD_MEM_IMM_REG1,
                .imm = 0x1LL + (~0x8LL),
                .scal = 0,
                .reg1 = (uint64_t)(&cpu_reg.rbp),
                .reg2 = 0},
            .dst = {
                .type = OD_REG,
                .imm = 0,
                .scal = 0,
                .reg1 = (uint64_t)(&cpu_reg.rax),
                .reg2 = 0
            }
        },
        // pop    %rbp
        {
            .op = INST_POP,
            .src = {
                .type = OD_REG,
                .imm = 0,
                .scal = 0,
                .reg1 = (uint64_t)(&cpu_reg.rbp),
                .reg2 = 0
            },
            .dst = {
                .type = OD_EMPTY,
                .imm = 0,
                .scal = 0,
                .reg1 = 0,
                .reg2 = 0
            }
        },
        // retq
        {
            .op = INST_RET,
            .src = {
                .type = OD_EMPTY,
                .imm = 0,
                .scal = 0,
                .reg1 = 0,
                .reg2 = 0
            },
            .dst = {
                .type = OD_EMPTY,
                .imm = 0,
                .scal = 0,
                .reg1 = 0,
                .reg2 = 0
            }
        },
        // mov    %rdx,%rsi
        {
            .op = INST_MOV,
            .src = {
                .type = OD_REG,
                .imm = 0,
                .scal = 0,
                .reg1 = (uint64_t)(&cpu_reg.rdx),
                .reg2 = 0
            },
            .dst = {
                .type = OD_REG,
                .imm = 0,
                .scal = 0,
                .reg1 = (uint64_t)(&cpu_reg.rsi),
                .reg2 = 0
            }
        },
        // mov    %rax,%rdi
        {
            .op = INST_MOV,
            .src = {
                .type = OD_REG,
                .imm = 0,
                .scal = 0,
                .reg1 = (uint64_t)(&cpu_reg.rax),
                .reg2 = 0
            },
            .dst = {
                .type = OD_REG,
                .imm = 0,
                .scal = 0,
                .reg1 = (uint64_t)(&cpu_reg.rdi),
                .reg2 = 0
            }
        },
        // callq  0
        {
            .op = INST_CALL,
            .src = {
                .type = OD_MEM_IMM,
                .imm = 0,
                .scal = 0,
                .reg1 = 0,
                .reg2 = 0
            },
            .dst = {
                .type = OD_EMPTY,
                .imm = 0,
                .scal = 0,
                .reg1 = 0,
                .reg2 = 0
            }
        },
        // mov    %rax,-0x8(%rbp)
        {
            .op = INST_MOV,
            .src = {
                .type = OD_REG,
                .imm = 0,
                .scal = 0,
                .reg1 = (uint64_t)(&cpu_reg.rax),
                .reg2 = 0
            },
            .dst = {
                .type = OD_MEM_IMM_REG1,
                .imm = 0x1LL + (~0x8LL),
                .scal = 0,
                .reg1 = (uint64_t)(&cpu_reg.rbp),
                .reg2 = 0
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

static void TestParsingOperand()
{
    printf("Testing operand parsing ...\n");

    const char *strs[11] = {
        "$0x1234",
        "%rax",
        "0xabcd",
        "(%rsp)",
        "0xabcd(%rsp)",
        "(%rsp,%rbx)",
        "0xabcd(%rsp,%rbx)",
        "(,%rbx,8)",
        "0xabcd(,%rbx,8)",
        "(%rsp,%rbx,8)",
        "0xabcd(%rsp,%rbx,8)",
    };
    
    od_t std_ods[11] = 
    {
        // $0x1234
        {
            .type   = OD_IMM,
            .imm    = 0x1234,
            .scal   = 0,
            .reg1   = 0,
            .reg2   = 0
        },
        // %rax
        {
            .type   = OD_REG,
            .imm    = 0,
            .scal   = 0,
            .reg1   = (uint64_t)(&cpu_reg.rax),
            .reg2   = 0
        },
        // 0xabcd
        {
            .type   = OD_MEM_IMM,
            .imm    = 0xabcd,
            .scal   = 0,
            .reg1   = 0,
            .reg2   = 0
        },
        // (%rsp)
        {
            .type   = OD_MEM_REG1,
            .imm    = 0,
            .scal   = 0,
            .reg1   = (uint64_t)(&cpu_reg.rsp),
            .reg2   = 0
        },
        // 0xabcd(%rsp)
        {
            .type   = OD_MEM_IMM_REG1,
            .imm    = 0xabcd,
            .scal   = 0,
            .reg1   = (uint64_t)(&cpu_reg.rsp),
            .reg2   = 0
        },
        // (%rsp,%rbx)
        {
            .type   = OD_MEM_REG1_REG2,
            .imm    = 0,
            .scal   = 0,
            .reg1   = (uint64_t)(&cpu_reg.rsp),
            .reg2   = (uint64_t)(&cpu_reg.rbx)
        },
        // 0xabcd(%rsp,%rbx)
        {
            .type   = OD_MEM_IMM_REG1_REG2,
            .imm    = 0xabcd,
            .scal   = 0,
            .reg1   = (uint64_t)(&cpu_reg.rsp),
            .reg2   = (uint64_t)(&cpu_reg.rbx)
        },
        // (,%rbx,8)
        {
            .type   = OD_MEM_REG2_SCAL,
            .imm    = 0,
            .scal   = 8,
            .reg1   = 0,
            .reg2   = (uint64_t)(&cpu_reg.rbx)
        },
        // 0xabcd(,%rbx,8)
        {
            .type   = OD_MEM_IMM_REG2_SCAL,
            .imm    = 0xabcd,
            .scal   = 8,
            .reg1   = 0,
            .reg2   = (uint64_t)(&cpu_reg.rbx)
        },
        // (%rsp,%rbx,8)
        {
            .type   = OD_MEM_REG1_REG2_SCAL,
            .imm    = 0,
            .scal   = 8,
            .reg1   = (uint64_t)(&cpu_reg.rsp),
            .reg2   = (uint64_t)(&cpu_reg.rbx)
        },
        // 0xabcd(%rsp,%rbx,8)
        {
            .type   = OD_MEM_IMM_REG1_REG2_SCAL,
            .imm    = 0xabcd,
            .scal   = 8,
            .reg1   = (uint64_t)(&cpu_reg.rsp),
            .reg2   = (uint64_t)(&cpu_reg.rbx)
        },
    };
    
    od_t od_parsed;

    for (int i = 0; i < 11; ++ i)
    {
        parse_operand(strs[i], &od_parsed);
        assert(operand_equal(&std_ods[i], &od_parsed) == 1);
    }

    printf("\033[32;1m\tPass\033[0m\n");
}

int main()
{
    TestParsingOperand();
    TestParsingInstruction();

    finally_cleanup();

    return 0;
}