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

/*======================================*/
/*      parse assembly instruction      */
/*======================================*/

// the implementation of ISA
extern void mov_handler             (od_t *src_od, od_t *dst_od);
extern void push_handler            (od_t *src_od, od_t *dst_od);
extern void pop_handler             (od_t *src_od, od_t *dst_od);
extern void leave_handler           (od_t *src_od, od_t *dst_od);
extern void call_handler            (od_t *src_od, od_t *dst_od);
extern void ret_handler             (od_t *src_od, od_t *dst_od);
extern void add_handler             (od_t *src_od, od_t *dst_od);
extern void sub_handler             (od_t *src_od, od_t *dst_od);
extern void cmp_handler             (od_t *src_od, od_t *dst_od);
extern void jne_handler             (od_t *src_od, od_t *dst_od);
extern void jmp_handler             (od_t *src_od, od_t *dst_od);
extern void lea_handler             (od_t *src_od, od_t *dst_od);
extern void int_handler             (od_t *src_od, od_t *dst_od);

static trie_node_t *register_mapping = NULL;
static trie_node_t *operator_mapping = NULL;
static void lazy_initialize_trie()
{
    // initialize the register mapping
    if (register_mapping == NULL)
    {
        register_mapping = trie_construct();
        register_mapping = trie_insert(register_mapping, "%rax",   (uint64_t)&(cpu_reg.rax)    );
        register_mapping = trie_insert(register_mapping, "%eax",   (uint64_t)&(cpu_reg.eax)    );
        register_mapping = trie_insert(register_mapping, "%ax",    (uint64_t)&(cpu_reg.ax)     );
        register_mapping = trie_insert(register_mapping, "%ah",    (uint64_t)&(cpu_reg.ah)     );
        register_mapping = trie_insert(register_mapping, "%al",    (uint64_t)&(cpu_reg.al)     );
        register_mapping = trie_insert(register_mapping, "%rbx",   (uint64_t)&(cpu_reg.rbx)    );
        register_mapping = trie_insert(register_mapping, "%ebx",   (uint64_t)&(cpu_reg.ebx)    );
        register_mapping = trie_insert(register_mapping, "%bx",    (uint64_t)&(cpu_reg.bx)     );
        register_mapping = trie_insert(register_mapping, "%bh",    (uint64_t)&(cpu_reg.bh)     );
        register_mapping = trie_insert(register_mapping, "%bl",    (uint64_t)&(cpu_reg.bl)     );
        register_mapping = trie_insert(register_mapping, "%rcx",   (uint64_t)&(cpu_reg.rcx)    );
        register_mapping = trie_insert(register_mapping, "%ecx",   (uint64_t)&(cpu_reg.ecx)    );
        register_mapping = trie_insert(register_mapping, "%cx",    (uint64_t)&(cpu_reg.cx)     );
        register_mapping = trie_insert(register_mapping, "%ch",    (uint64_t)&(cpu_reg.ch)     );
        register_mapping = trie_insert(register_mapping, "%cl",    (uint64_t)&(cpu_reg.cl)     );
        register_mapping = trie_insert(register_mapping, "%rdx",   (uint64_t)&(cpu_reg.rdx)    );
        register_mapping = trie_insert(register_mapping, "%edx",   (uint64_t)&(cpu_reg.edx)    );
        register_mapping = trie_insert(register_mapping, "%dx",    (uint64_t)&(cpu_reg.dx)     );
        register_mapping = trie_insert(register_mapping, "%dh",    (uint64_t)&(cpu_reg.dh)     );
        register_mapping = trie_insert(register_mapping, "%dl",    (uint64_t)&(cpu_reg.dl)     );
        register_mapping = trie_insert(register_mapping, "%rsi",   (uint64_t)&(cpu_reg.rsi)    );
        register_mapping = trie_insert(register_mapping, "%esi",   (uint64_t)&(cpu_reg.esi)    );
        register_mapping = trie_insert(register_mapping, "%si",    (uint64_t)&(cpu_reg.si)     );
        register_mapping = trie_insert(register_mapping, "%sih",   (uint64_t)&(cpu_reg.sih)    );
        register_mapping = trie_insert(register_mapping, "%sil",   (uint64_t)&(cpu_reg.sil)    );
        register_mapping = trie_insert(register_mapping, "%rdi",   (uint64_t)&(cpu_reg.rdi)    );
        register_mapping = trie_insert(register_mapping, "%edi",   (uint64_t)&(cpu_reg.edi)    );
        register_mapping = trie_insert(register_mapping, "%di",    (uint64_t)&(cpu_reg.di)     );
        register_mapping = trie_insert(register_mapping, "%dih",   (uint64_t)&(cpu_reg.dih)    );
        register_mapping = trie_insert(register_mapping, "%dil",   (uint64_t)&(cpu_reg.dil)    );
        register_mapping = trie_insert(register_mapping, "%rbp",   (uint64_t)&(cpu_reg.rbp)    );
        register_mapping = trie_insert(register_mapping, "%ebp",   (uint64_t)&(cpu_reg.ebp)    );
        register_mapping = trie_insert(register_mapping, "%bp",    (uint64_t)&(cpu_reg.bp)     );
        register_mapping = trie_insert(register_mapping, "%bph",   (uint64_t)&(cpu_reg.bph)    );
        register_mapping = trie_insert(register_mapping, "%bpl",   (uint64_t)&(cpu_reg.bpl)    );
        register_mapping = trie_insert(register_mapping, "%rsp",   (uint64_t)&(cpu_reg.rsp)    );
        register_mapping = trie_insert(register_mapping, "%esp",   (uint64_t)&(cpu_reg.esp)    );
        register_mapping = trie_insert(register_mapping, "%sp",    (uint64_t)&(cpu_reg.sp)     );
        register_mapping = trie_insert(register_mapping, "%sph",   (uint64_t)&(cpu_reg.sph)    );
        register_mapping = trie_insert(register_mapping, "%spl",   (uint64_t)&(cpu_reg.spl)    );
        register_mapping = trie_insert(register_mapping, "%r8",    (uint64_t)&(cpu_reg.r8)     );
        register_mapping = trie_insert(register_mapping, "%r8d",   (uint64_t)&(cpu_reg.r8d)    );
        register_mapping = trie_insert(register_mapping, "%r8w",   (uint64_t)&(cpu_reg.r8w)    );
        register_mapping = trie_insert(register_mapping, "%r8b",   (uint64_t)&(cpu_reg.r8b)    );
        register_mapping = trie_insert(register_mapping, "%r9",    (uint64_t)&(cpu_reg.r9)     );
        register_mapping = trie_insert(register_mapping, "%r9d",   (uint64_t)&(cpu_reg.r9d)    );
        register_mapping = trie_insert(register_mapping, "%r9w",   (uint64_t)&(cpu_reg.r9w)    );
        register_mapping = trie_insert(register_mapping, "%r9b",   (uint64_t)&(cpu_reg.r9b)    );
        register_mapping = trie_insert(register_mapping, "%r10",   (uint64_t)&(cpu_reg.r10)    );
        register_mapping = trie_insert(register_mapping, "%r10d",  (uint64_t)&(cpu_reg.r10d)   );
        register_mapping = trie_insert(register_mapping, "%r10w",  (uint64_t)&(cpu_reg.r10w)   );
        register_mapping = trie_insert(register_mapping, "%r10b",  (uint64_t)&(cpu_reg.r10b)   );
        register_mapping = trie_insert(register_mapping, "%r11",   (uint64_t)&(cpu_reg.r11)    );
        register_mapping = trie_insert(register_mapping, "%r11d",  (uint64_t)&(cpu_reg.r11d)   );
        register_mapping = trie_insert(register_mapping, "%r11w",  (uint64_t)&(cpu_reg.r11w)   );
        register_mapping = trie_insert(register_mapping, "%r11b",  (uint64_t)&(cpu_reg.r11b)   );
        register_mapping = trie_insert(register_mapping, "%r12",   (uint64_t)&(cpu_reg.r12)    );
        register_mapping = trie_insert(register_mapping, "%r12d",  (uint64_t)&(cpu_reg.r12d)   );
        register_mapping = trie_insert(register_mapping, "%r12w",  (uint64_t)&(cpu_reg.r12w)   );
        register_mapping = trie_insert(register_mapping, "%r12b",  (uint64_t)&(cpu_reg.r12b)   );
        register_mapping = trie_insert(register_mapping, "%r13",   (uint64_t)&(cpu_reg.r13)    );
        register_mapping = trie_insert(register_mapping, "%r13d",  (uint64_t)&(cpu_reg.r13d)   );
        register_mapping = trie_insert(register_mapping, "%r13w",  (uint64_t)&(cpu_reg.r13w)   );
        register_mapping = trie_insert(register_mapping, "%r13b",  (uint64_t)&(cpu_reg.r13b)   );
        register_mapping = trie_insert(register_mapping, "%r14",   (uint64_t)&(cpu_reg.r14)    );
        register_mapping = trie_insert(register_mapping, "%r14d",  (uint64_t)&(cpu_reg.r14d)   );
        register_mapping = trie_insert(register_mapping, "%r14w",  (uint64_t)&(cpu_reg.r14w)   );
        register_mapping = trie_insert(register_mapping, "%r14b",  (uint64_t)&(cpu_reg.r14b)   );
        register_mapping = trie_insert(register_mapping, "%r15",   (uint64_t)&(cpu_reg.r15)    );
        register_mapping = trie_insert(register_mapping, "%r15d",  (uint64_t)&(cpu_reg.r15d)   );
        register_mapping = trie_insert(register_mapping, "%r15w",  (uint64_t)&(cpu_reg.r15w)   );
        register_mapping = trie_insert(register_mapping, "%r15b",  (uint64_t)&(cpu_reg.r15b)   );
    }

    // initialize the operator mapping
    if (operator_mapping == NULL)
    {
        operator_mapping = trie_construct();
        operator_mapping = trie_insert(operator_mapping, "movq",   (uint64_t)&mov_handler    );
        operator_mapping = trie_insert(operator_mapping, "mov",    (uint64_t)&mov_handler    );
        operator_mapping = trie_insert(operator_mapping, "push",   (uint64_t)&push_handler   );
        operator_mapping = trie_insert(operator_mapping, "pop",    (uint64_t)&pop_handler    );
        operator_mapping = trie_insert(operator_mapping, "leaveq", (uint64_t)&leave_handler  );
        operator_mapping = trie_insert(operator_mapping, "callq",  (uint64_t)&call_handler   );
        operator_mapping = trie_insert(operator_mapping, "retq",   (uint64_t)&ret_handler    );
        operator_mapping = trie_insert(operator_mapping, "add",    (uint64_t)&add_handler    );
        operator_mapping = trie_insert(operator_mapping, "sub",    (uint64_t)&sub_handler    );
        operator_mapping = trie_insert(operator_mapping, "cmpq",   (uint64_t)&cmp_handler    );
        operator_mapping = trie_insert(operator_mapping, "jne",    (uint64_t)&jne_handler    );
        operator_mapping = trie_insert(operator_mapping, "jmp",    (uint64_t)&jmp_handler    );
        operator_mapping = trie_insert(operator_mapping, "lea",    (uint64_t)&lea_handler    );
        operator_mapping = trie_insert(operator_mapping, "int",    (uint64_t)&int_handler    );
    }
}

typedef enum
{
    INST_PARSE_START,
    
    // parsing operator
    INST_PARSE_OPERATOR,
    
    // parsing src operand
    INST_PARSE_SPACE_SRC_OPERAND,
    INST_PARSE_SRC_OPERAND,
    
    // parsing dst operand
    INST_PARSE_SPACE_DST_OPERAND,
    INST_PARSE_DST_OPERAND,

    INST_PARSE_PARSED,
} inst_parse_state_t;

typedef enum
{
    // starting
    OPERAND_PARSE_START,

    // parsing imm, reg, or mem
    OPERAND_PARSE_IMM,
    OPERAND_PARSE_REG,
    OPERAND_PARSE_MEM,

    OPERAND_PARSE_PARSED,
} operand_parse_state_t;

typedef enum
{
    // starting
    MEM_PARSE_START,

    // imm...
    MEM_PARSE_IMM,
    // *(...
    MEM_PARSE_LEFT_PARENTHESIS,
    // *(reg1...
    MEM_PARSE_FIRST_REGISTER,
    // *(*,reg2...
    MEM_PARSE_SECOND_REGISTER,
    // *(*,reg2,scale...
    MEM_PARSE_SCALE,
    // *(*,reg2,[1,2,4,8]
    MEM_PARSE_SCALE_PARSED,
    // *)
    MEM_PARSE_RIGHT_PARENTHESIS,
    // *
    MEM_PARSE_PARSED,
} mem_parse_state_t;

typedef struct
{
    // parser for string
    trie_node_t *trie_node;

    // parser for number
    string2uint_state_t imm_state;

    // parser state
    inst_parse_state_t inst_state;

    // operand parse state
    operand_parse_state_t od_state;

    // effective parse state
    mem_parse_state_t mem_state;

    // parsed effective address
    uint64_t imm;
    uint64_t reg1;
    uint64_t reg2;
    uint64_t scal;

    // parsed operand
    od_t operand;

    // parsed result
    inst_t *inst;
} inst_parser_t;

static inst_parser_t *parse_instruction_next(inst_parser_t *p, char c);
static inst_parser_t *parse_operand_next(inst_parser_t *p, char c);
static inst_parser_t *parse_effective_address_next(inst_parser_t *p, char c);

// DFA to parse instruction in one-time left-right scanning
static inst_parser_t *parse_instruction_next(inst_parser_t *p, char c)
{
    //  p       the parser
    //  c       the input character
    assert(p != NULL);

    switch (p->inst_state)
    {
        case INST_PARSE_START:
            if ('a' <= c && c <= 'z')
            {
                // start parsing operator
                p->trie_node = operator_mapping;
                // accepting first char in operator
                p->trie_node = trie_next(p->trie_node, c);
                assert(p->trie_node != NULL);
                p->inst_state = INST_PARSE_OPERATOR;
                return p;
            }
            else if (c == ' ' || c == '\t' || c == '\r')
            {
                // skip leading spaces
                return p;
            }
            assert(0);
        case INST_PARSE_OPERATOR:
            if ('a' <= c && c <= 'z')
            {
                p->trie_node = trie_next(p->trie_node, c);
                assert(p->trie_node != NULL);
                return p;
            }
            else if (c == ' ' || c == '\t' || c == '\r')
            {
                // operator parsed
                assert(p->trie_node != NULL);
                assert(p->trie_node->isvalue == 1);
                // get operator
                p->inst->op = (op_t)p->trie_node->value;

                // transfer to first operand
                p->inst_state = INST_PARSE_SPACE_SRC_OPERAND;
                return p;
            }
            else if (c == '\n')
            {
                // instruction ends without operand like: `NOP`, `RET`
                assert(p->trie_node != NULL);
                assert(p->trie_node->isvalue == 1);
                // get operator
                p->inst->op = (op_t)p->trie_node->value;

                p->inst_state = INST_PARSE_PARSED;
                p->inst->src.type = OD_EMPTY;
                p->inst->src.value = 0;
                p->inst->dst.type = OD_EMPTY;
                p->inst->dst.value = 0;
                return p;
            }
            assert(0);
        case INST_PARSE_SPACE_SRC_OPERAND:
            // spaces between operator and src operand
            if (c == '$' ||     // immediate number
                c == '%' ||     // register
                (c == '(' || ('0' <= c && c <= '9') || c == '-')    // effective address
            )
            {
                // start parsing operand
                p->inst_state = INST_PARSE_SRC_OPERAND;
                p->od_state = OPERAND_PARSE_START;
                return parse_operand_next(p, c);
            }
            else if (c == ' ' || c == '\t' || c == '\r')
            {
                // processing spaces
                return p;
            }
            else if (c == '\n')
            {
                // instruction ends without operand like: `NOP`, `RET`
                p->inst_state = INST_PARSE_PARSED;
                p->inst->src.type = OD_EMPTY;
                p->inst->src.value = 0;
                p->inst->dst.type = OD_EMPTY;
                p->inst->dst.value = 0;
                return p;
            }
            assert(0);
        case INST_PARSE_SRC_OPERAND:
            // parsing src operand
            p = parse_operand_next(p, c);
            if (p->od_state == OPERAND_PARSE_PARSED)
            {
                // src parsed
                // copy the result to src
                p->inst->src.type = p->operand.type;
                p->inst->src.value = p->operand.value;

                // going to parse dst
                if (c == '\n')
                {
                    p->inst_state = INST_PARSE_PARSED;
                    p->inst->dst.type = OD_EMPTY;
                    p->inst->dst.value = 0;
                }
                else
                {
                    p->inst_state = INST_PARSE_SPACE_DST_OPERAND;
                }
            }
            return p;
        case INST_PARSE_SPACE_DST_OPERAND:
            // spaces between operator and src operand
            if (c == '$' ||     // immediate number
                c == '%' ||     // register
                (c == '(' || ('0' <= c && c <= '9') || c == '-')    // effective address
            )
            {
                // start parsing operand
                p->inst_state = INST_PARSE_DST_OPERAND;
                p->od_state = OPERAND_PARSE_START;
                return parse_operand_next(p, c);
            }
            else if (c == ' ' || c == '\t' || c == '\r')
            {
                // processing spaces
                return p;
            }
            else if (c == '\n')
            {
                // instruction ends without dst operand
                p->inst_state = INST_PARSE_PARSED;
                p->inst->dst.type = OD_EMPTY;
                p->inst->dst.value = 0;
                return p;
            }
            assert(0);
        case INST_PARSE_DST_OPERAND:
            // parsing dst operand
            p = parse_operand_next(p, c);
            if (p->od_state == OPERAND_PARSE_PARSED)
            {
                // dst parsed
                // copy the result to dst
                p->inst->dst.type = p->operand.type;
                p->inst->dst.value = p->operand.value;

                // going to parse dst
                p->inst_state = INST_PARSE_PARSED;
            }
            return p;
        default:
            assert(0);
    }
}

// DFA to parse operand in one-time left-right scanning
static inst_parser_t *parse_operand_next(inst_parser_t *p, char c)
{
    //  p       the parser
    //  c       the input character
    assert(p != NULL);

    switch (p->od_state)
    {
        case OPERAND_PARSE_START:
            // the start of parsing operands
            if (c == '$')
            {
                // immediate number
                // start parsing immediate number
                // DFA: string2uint_next
                p->imm_state = STRING2UINT_LEADING_SPACE;
                p->imm = 0;
                p->od_state = OPERAND_PARSE_IMM;
                return p;
            }
            else if (c == '%')
            {
                // register
                // start parsing register
                p->trie_node = register_mapping;
                // accepting first char in register ('%')
                p->trie_node = trie_next(p->trie_node, c);
                assert(p->trie_node != NULL);
                p->od_state = OPERAND_PARSE_REG;
                return p;
            }
            else if (('0' <= c && c <= '9') || c == '-' || c == '(')
            {
                // effective address
                // start parsing effective address
                p->od_state = OPERAND_PARSE_MEM;
                p->mem_state = MEM_PARSE_START;
                p = parse_effective_address_next(p, c);
                return p;
            }
            assert(0);
        case OPERAND_PARSE_IMM:
            // immediate number
            // use string2uint_next as DFA to parse immediate number
            if (('0' <= c && c <= '9') || 
                ('a' <= c && c <= 'f') || 
                ('A' <= c && c <= 'F') || 
                c == 'x' || c == 'X' || c == '-')
            {
                // still immediate number
                p->imm_state = string2uint_next(p->imm_state, c, &(p->imm));
                if (p->imm_state != STRING2UINT_FAILED)
                {
                    return p;
                }
            }
            else if (c == ',' || c == ' ' || c == '\t' || c == '\r' || c == '\n')
            {
                // end of parsing this operand: imm
                p->od_state = OPERAND_PARSE_PARSED;
                p->operand.type = OD_IMM;
                p->operand.value = p->imm;
                return p;
            }
            assert(0);
        case OPERAND_PARSE_REG:
            // register
            if ('a' <= c && c <= 'z')
            {
                // still a register
                p->trie_node = trie_next(p->trie_node, c);
                assert(p->trie_node != NULL);
                return p;
            }
            else if (c == ',' || c == ' ' || c == '\t' || c == '\r' || c == '\n')
            {
                // end of parsing this operand: reg
                p->od_state = OPERAND_PARSE_PARSED;
                assert(p->trie_node->isvalue == 1);
                p->operand.type = OD_REG;
                p->operand.value = p->trie_node->value;
                return p;
            }
            assert(0);
        case OPERAND_PARSE_MEM:
            p = parse_effective_address_next(p, c);
            if (p->mem_state == MEM_PARSE_PARSED)
            {
                p->od_state = OPERAND_PARSE_PARSED;
            }
            return p;
        default:
            assert(0);
    }
}

uint64_t zero_register = 0;

// DFA to parse effective address in one-time left-right scanning
static inst_parser_t *parse_effective_address_next(inst_parser_t *p, char c)
{
    // p    the parser
    // c    the input character
    assert(p != NULL);

    switch (p->mem_state)
    {
        case MEM_PARSE_START:
            // start parsing effective address
            if (('0' <= c && c <= '9') || c == '-')
            {
                // prefix immediate number
                p->imm_state = STRING2UINT_LEADING_SPACE;
                p->imm_state = string2uint_next(p->imm_state, c, &(p->imm));
                if (p->imm_state != STRING2UINT_FAILED)
                {
                    p->mem_state = MEM_PARSE_IMM;
                    return p;
                }
            }
            else if (c == '(')
            {
                // no prefix immediate number
                p->imm = 0;
                p->mem_state = MEM_PARSE_LEFT_PARENTHESIS;
                return p;
            }
            assert(0);
        case MEM_PARSE_IMM:
            // parsing the immediate number
            if (('0' <= c && c <= '9') || 
                ('a' <= c && c <= 'f') || 
                ('A' <= c && c <= 'F') || 
                c == 'x' || c == 'X')
            {
                p->imm_state = string2uint_next(p->imm_state, c, &(p->imm));
                if (p->imm_state != STRING2UINT_FAILED)
                {
                    return p;
                }
            }
            else if (c == '(')
            {
                // successfully parsed immediate number
                p->mem_state = MEM_PARSE_LEFT_PARENTHESIS;
                return p;
            }
            else if (c == ',' || c == ' ' ||  c == '\t' || c == '\r' || c == '\n')
            {
                // end of parsing this operand: reg
                p->mem_state = MEM_PARSE_PARSED;
                // computing of effective address
                // this is the work of ALU
                p->operand.type = OD_MEM;
                p->operand.value = p->imm;
                return p;
            }
            assert(0);
        case MEM_PARSE_LEFT_PARENTHESIS:
            // *(...
            if (c == '%')
            {
                // *(reg1
                // parsing reg1, '%' accepted
                p->trie_node = register_mapping;
                p->trie_node = trie_next(p->trie_node, c);
                assert(p->trie_node != NULL);
                p->mem_state = MEM_PARSE_FIRST_REGISTER;
                return p;
            }
            else if (c == ',')
            {
                // *(,reg2...
                // initialize the second register
                // and we have not accepted '%' here
                p->trie_node = register_mapping;
                p->reg1 = (uint64_t)&zero_register;
                p->mem_state = MEM_PARSE_SECOND_REGISTER;
                return p;
            }
            assert(0);
        case MEM_PARSE_FIRST_REGISTER:
            // *(reg1...
            if ('a' <= c && c <= 'z')
            {
                // parsing reg1
                p->trie_node = trie_next(p->trie_node, c);
                assert(p->trie_node != NULL);
                return p;
            }
            else if (c == ',')
            {
                // end of parsing reg1
                assert(p->trie_node->isvalue == 1);
                p->reg1 = p->trie_node->value;
                // initialize the second register
                // and we have not accepted '%' here
                p->trie_node = register_mapping;
                p->mem_state = MEM_PARSE_SECOND_REGISTER;
                return p;
            }
            else if (c == ')')
            {
                // end of parsing reg1
                assert(p->trie_node->isvalue == 1);
                p->reg1 = p->trie_node->value;
                p->mem_state = MEM_PARSE_RIGHT_PARENTHESIS;
                
                // computing of effective address
                p->operand.type = OD_MEM;
                p->operand.value = *(uint64_t *)p->reg1 + p->imm;
                return p;
            }
            assert(0);
        case MEM_PARSE_SECOND_REGISTER:
            // `zero_register` is making the effective address safe to compute
            // even without reg1, the reg1 is set to zero_register
            // so the we can get 0 from reg1
            if (c == '%' || ('a' <= c && c <= 'z'))
            {
                // parsing reg2
                p->trie_node = trie_next(p->trie_node, c);
                assert(p->trie_node != NULL);
                return p;
            }
            else if (c == ',')
            {
                // reg2 parsed
                assert(p->trie_node->isvalue == 1);
                p->reg2 = p->trie_node->value;
                // going to parse scale
                p->mem_state = MEM_PARSE_SCALE;
                return p;
            }
            else if (c == ')')
            {
                // *(*,reg2)
                // reg2 parsed
                assert(p->trie_node->isvalue == 1);
                p->reg2 = p->trie_node->value;
                p->scal = 1;
                p->mem_state = MEM_PARSE_RIGHT_PARENTHESIS;

                // computing of effective address
                p->operand.type = OD_MEM;
                p->operand.value = *(uint64_t *)p->reg2 + *(uint64_t *)p->reg1 + p->imm;
                return p;
            }
            assert(0);
        case MEM_PARSE_SCALE:
            if (c == '1' || c == '2' || c == '4' || c == '8')
            {
                p->scal = c - '0';
                p->od_state = MEM_PARSE_SCALE_PARSED;

                // computing of effective address
                p->operand.type = OD_MEM;
                p->operand.value = *(uint64_t *)p->reg2 * p->scal + *(uint64_t *)p->reg1 + p->imm;
                return p;
            }
            assert(0);
        case MEM_PARSE_SCALE_PARSED:
            if (c == ')')
            {
                p->od_state = MEM_PARSE_RIGHT_PARENTHESIS;
                return p;
            }
            assert(0);
        case MEM_PARSE_RIGHT_PARENTHESIS:
            if (c == ',' || c == ' ' ||  c == '\t' || c == '\r' || c == '\n')
            {
                // end of parsing this operand: reg
                p->mem_state = MEM_PARSE_PARSED;
                return p;
            }
            assert(0);
        default:
            assert(0);
    }
}

void parse_instruction(char *inst_str, inst_t *inst)
{
    lazy_initialize_trie();

    inst_parser_t parser =
    {
        .inst = inst
    };
    inst_parser_t *p = &parser;
    
    for (int i = 0; i < strlen(inst_str); ++ i)
    {
        p = parse_instruction_next(p, inst_str[i]);

        if (p->inst_state == INST_PARSE_PARSED)
        {
            break;
        }
    }
    p = parse_instruction_next(p, '\n');
    assert(p->inst_state == INST_PARSE_PARSED);
}