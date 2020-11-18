#ifndef inst_guard
#define inst_guard

#include<stdlib.h>
#include<stdint.h>

#define NUM_INSTRTYPE 30

typedef enum OP 
{
    mov_reg_reg,        // 0 
    mov_reg_mem,        // 1
    mov_mem_reg,        // 2
    push_reg,           // 3
    pop_reg,            // 4
    call,               // 5
    ret,                // 6
    add_reg_reg,        // 7
} op_t;

typedef enum OD_TYPE
{
    EMPTY,
    IMM, REG, 
    MM_IMM, MM_REG, MM_IMM_REG, MM_REG1_REG2, 
    MM_IMM_REG1_REG2, MM_REG2_S, MM_IMM_REG2_S, 
    MM_REG1_REG2_S, MM_IMM_REG1_REG2_S
} od_type_t;

typedef struct OD
{
    od_type_t type;

    int64_t imm;
    int64_t scal;
    uint64_t *reg1;
    uint64_t *reg2;
} od_t;

typedef struct INSTRUCT_STRUCT
{
    op_t op;   // mov, push
    od_t src; 
    od_t dst;
    char code[100];
} inst_t;

// pointer pointing to the function
typedef void (*handler_t)(uint64_t, uint64_t);

handler_t handler_table[NUM_INSTRTYPE];

void init_handler_table();

void instruction_cycle();

void mov_reg_reg_handler(uint64_t src, uint64_t dst);
void call_handler(uint64_t src, uint64_t dst);
void add_reg_reg_handler(uint64_t src, uint64_t dst);
void push_reg_handler(uint64_t src, uint64_t dst);
void pop_reg_handler(uint64_t src, uint64_t dst);
void mov_reg_mem_handler(uint64_t src, uint64_t dst);

#endif