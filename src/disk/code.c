#include <stdlib.h>

#include "cpu/register.h"
#include "memory/instruction.h"

inst_t program[15] = 
{
    // uint64_t add(uint64_t, uint64_t)
    {
        push_reg,
        {   REG,    0,  0,  (uint64_t *)&reg.rbp,   NULL  },
        {   EMPTY,  0,  0,  NULL,                   NULL  },
        "push   \%rbp"
    },
    {
        mov_reg_reg,
        {   REG,  0,  0,  (uint64_t *)&reg.rsp,     NULL  },
        {   REG,  0,  0,  (uint64_t *)&reg.rbp,     NULL  },
        "mov    \%rsp,\%rbp"
    },
    {
        mov_reg_mem,
        {   REG,  0,  0,  (uint64_t *)&reg.rdi,     NULL  },
        {   MM_IMM_REG,  -0x18,  0,  (uint64_t *)&reg.rbp,     NULL  },
        "mov    \%rdi,-0x18(\%rbp)"
    },
    {
        mov_reg_mem,
        {   REG,  0,  0,  (uint64_t *)&reg.rsi,     NULL  },
        {   MM_IMM_REG,  -0x20,  0,  (uint64_t *)&reg.rbp,     NULL  },
        "mov    \%rsi,-0x20(\%rbp)"
    },
    // MARK
    {
        mov_mem_reg,
        {   MM_IMM_REG,  -0x18,  0,  (uint64_t *)&reg.rbp,     NULL  },
        {   REG,  0,  0,  (uint64_t *)&reg.rdx,     NULL  },
        "mov    -0x18(\%rbp),\%rdx"
    },
    {
        mov_mem_reg,
        {   MM_IMM_REG,  -0x20,  0,  (uint64_t *)&reg.rbp,     NULL  },
        {   REG,  0,  0,  (uint64_t *)&reg.rax,     NULL  },
        "mov    -0x20(\%rbp),\%rax"
    },
    {
        add_reg_reg,
        {   REG,  0,  0,  (uint64_t *)&reg.rdx,     NULL  },
        {   REG,  0,  0,  (uint64_t *)&reg.rax,     NULL  },
        "add    \%rdx,\%rax"
    },
    {
        mov_reg_mem,
        {   REG,  0,  0,  (uint64_t *)&reg.rax,     NULL  },
        {   MM_IMM_REG,  -0x8,  0,  (uint64_t *)&reg.rbp,     NULL  },
        "mov    \%rax,-0x8(\%rbp)"
    },
    {
        mov_mem_reg,
        {   MM_IMM_REG,  -0x8,  0,  (uint64_t *)&reg.rbp,     NULL  },
        {   REG,  0,  0,  (uint64_t *)&reg.rax,     NULL  },
        "mov    -0x8(\%rbp),\%rax"
    },
    {
        pop_reg,
        {   REG,    0,  0,  (uint64_t *)&reg.rbp,   NULL  },
        {   EMPTY,  0,  0,  NULL,                   NULL  },
        "pop   \%rbp"
    },
    {
        ret,
        {   EMPTY,  0,  0,  NULL,   NULL  },
        {   EMPTY,  0,  0,  NULL,   NULL  },
        "retq"
    },
    // main entry point
    {
        mov_reg_reg,
        { REG, 0, 0, &reg.rdx, NULL },
        { REG, 0, 0, &reg.rsi, NULL },
        "mov    \%rdx,\%rsi"
    },
    {
        mov_reg_reg,
        {   REG,  0,  0,  (uint64_t *)&reg.rax,     NULL  },
        {   REG,  0,  0,  (uint64_t *)&reg.rdi,     NULL  },
        "mov    \%rax,\%rdi"
    },
    { // reg.rip
        call,
        {   IMM,    (uint64_t)&(program[0]),  0,  NULL,     NULL  },
        {   EMPTY,  0,  0,  NULL,     NULL  },
        "callq  <add>"
    },
    { // return address
        mov_reg_mem,
        {   REG,  0,  0,  (uint64_t *)&reg.rax,     NULL  },
        {   MM_IMM_REG,  -0x8,  0,  (uint64_t *)&reg.rbp,     NULL  },
        "mov    \%rax,-0x8(\%rbp)"
    }
};