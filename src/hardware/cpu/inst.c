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

static trie_node_t *register_mapping = NULL;
static trie_node_t *operator_mapping = NULL;

static void trie_cleanup()
{
    trie_free(register_mapping);
    trie_free(operator_mapping);
}

static void lazy_initialize_trie()
{
    // initialize the register mapping
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

    // initialize the operator mapping
    operator_mapping = trie_construct();
    operator_mapping= trie_insert(operator_mapping, "movq",   INST_MOV    );
    operator_mapping= trie_insert(operator_mapping, "mov",    INST_MOV    );
    operator_mapping= trie_insert(operator_mapping, "push",   INST_PUSH   );
    operator_mapping= trie_insert(operator_mapping, "pop",    INST_POP    );
    operator_mapping= trie_insert(operator_mapping, "leaveq", INST_LEAVE  );
    operator_mapping= trie_insert(operator_mapping, "callq",  INST_CALL   );
    operator_mapping= trie_insert(operator_mapping, "retq",   INST_RET    );
    operator_mapping= trie_insert(operator_mapping, "add",    INST_ADD    );
    operator_mapping= trie_insert(operator_mapping, "sub",    INST_SUB    );
    operator_mapping= trie_insert(operator_mapping, "cmpq",   INST_CMP    );
    operator_mapping= trie_insert(operator_mapping, "jne",    INST_JNE    );
    operator_mapping= trie_insert(operator_mapping, "jmp",    INST_JMP    );
    operator_mapping= trie_insert(operator_mapping, "lea",    INST_LEA    );

    // add the cleanup events
    add_cleanup_event(&trie_cleanup);
}

static uint64_t try_get_from_trie(trie_node_t **root, char *key)
{
    if (*root == NULL)
    {
        lazy_initialize_trie();
    }
    uint64_t val;
    int result = trie_get(*root, key, &val);
    if (result == 0)
    {
        printf("could not find key '%s' from trie\n", key);
        exit(0);
    }
    return val;
}

// functions to map the string assembly code to inst_t instance
void parse_instruction(const char *str, inst_t *inst);
void parse_operand(const char *str, od_t *od);
uint64_t compute_operand(od_t *od);

// interpret the operand
uint64_t compute_operand(od_t *od)
{
    if (od->type == OD_IMM)
    {
        // immediate signed number can be negative: convert to bitmap
        return *(uint64_t *)&od->imm;
    }
    else if (od->type == OD_REG)
    {
        // default register 1
        return od->reg1;
    }
    else if (od->type == OD_EMPTY)
    {
        return 0;
    }
    else
    {
        // access memory: return the physical address
        uint64_t vaddr = 0;

        if (od->type == OD_MEM_IMM)
        {
            vaddr = od->imm;
        }
        else if (od->type == OD_MEM_REG1)
        {
            vaddr = *((uint64_t *)od->reg1);
        }
        else if (od->type == OD_MEM_IMM_REG1)
        {
            vaddr = od->imm + (*((uint64_t *)od->reg1));
        }
        else if (od->type == OD_MEM_REG1_REG2)
        {
            vaddr = (*((uint64_t *)od->reg1)) + (*((uint64_t *)od->reg2));
        }
        else if (od->type == OD_MEM_IMM_REG1_REG2)
        {
            vaddr = od->imm +  (*((uint64_t *)od->reg1)) + (*((uint64_t *)od->reg2));
        }
        else if (od->type == OD_MEM_REG2_SCAL)
        {
            vaddr = (*((uint64_t *)od->reg2)) * od->scal;
        }
        else if (od->type == OD_MEM_IMM_REG2_SCAL)
        {
            vaddr = od->imm + (*((uint64_t *)od->reg2)) * od->scal;
        }
        else if (od->type == OD_MEM_REG1_REG2_SCAL)
        {
            vaddr = (*((uint64_t *)od->reg1)) + (*((uint64_t *)od->reg2)) * od->scal;
        }
        else if (od->type == OD_MEM_IMM_REG1_REG2_SCAL)
        {
            vaddr = od->imm + (*((uint64_t *)od->reg1)) + (*((uint64_t *)od->reg2)) * od->scal;
        }
        return vaddr;
    }

    // empty
    return 0;
}

typedef enum
{
    LEADING_SPACE,
    
    // parsing operator
    OPERATOR,
    
    SPACE_SRC_OPERAND,
    // parsing src operand
    SRC_OPERAND,
    OPERAND_IMM,
    OPERAND_REG,
    OPERAND_MEM,
    
    SPACE_DST_OPERAND,
    // parsing dst operand
    DST_OPERAND,
    
    // failure
    FAILED_TRANSFER,
} inst_parse_state_t;

// DFA to parse instruction in one-time left-right scanning
inst_parse_state_t parse_instruction_next(inst_parse_state_t state, char c, inst_t *inst,
    trie_node_t **trie_data_ptr, string2uint_state_t *num_state)
{
    //  state               the current state of parsing
    //  c                   the input character
    //  inst                the output inst_t structure (result container)
    //  trie_data_ptr       the pointer to .data section, finally points to heap
    assert(inst != NULL);

    switch (state)
    {
        case LEADING_SPACE:
            if ('a' <= c && c <= 'z')
            {
                // start parsing operator
                *trie_data_ptr = &operator_mapping;
                // accepting first char in operator
                *trie_data_ptr = trie_next(*trie_data_ptr, c);
                if (*trie_data_ptr == NULL)
                {
                    return FAILED_TRANSFER;
                }
                return OPERATOR;
            }
            else if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            {
                // skip leading spaces
                return LEADING_SPACE;
            }
            return FAILED_TRANSFER;
        case OPERATOR:
            if ('a' <= c && c <= 'z')
            {
                assert(*trie_data_ptr != NULL);
                *trie_data_ptr = trie_next(*trie_data_ptr, c);
                if (*trie_data_ptr == NULL)
                {
                    return FAILED_TRANSFER;
                }
                return OPERATOR;
            }
            else if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            {
                // operator parsed
                assert(*trie_data_ptr != NULL);
                assert((*trie_data_ptr)->isvalue == 1);
                // get operator
                inst->op = (op_t)(*trie_data_ptr)->value;

                // transfer to first operand
                return SPACE_SRC_OPERAND;
            }
            return FAILED_TRANSFER;
        case SPACE_SRC_OPERAND:
            // spaces between operator and src operand
            if (c == '$' ||                             // immediate number
                c == '%' ||                             // register
                (c == '(' || ('0' <= c && c <= '9'))    // memory
            )
            {
                // start parsing operator
                return parse_operand_next(state, c, &inst->src, trie_data_ptr, num_state);
            }
            else if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            {
                return SPACE_SRC_OPERAND;
            }
            return FAILED_TRANSFER;
        default:
            return FAILED_TRANSFER;
    }
}

// DFA to parse operand in one-time left-right scanning
inst_parse_state_t parse_operand_next(inst_parse_state_t state, char c, od_t *od,
    trie_node_t **trie_data_ptr, string2uint_state_t *num_state)
{
    //  state               the current state of parsing
    //  c                   the input character
    //  inst                the output inst_t structure (result container)
    //  trie_data_ptr       the pointer to .data section, finally points to heap
    assert(od != NULL);

    switch (state)
    {
        case SRC_OPERAND:
        case DST_OPERAND:
            // the start of parsing operands
            if (c == '$')
            {
                // immediate number
                // start parsing immediate number
                // DFA: string2uint_next
                *num_state = LEADING_SPACE;
                od->type = OD_IMM;
                od->imm = 0;
            }
            else if (c == '%')
            {
                // register
                // start parsing register
                od->type = OD_REG;
                *trie_data_ptr = &register_mapping;
                // accepting first char in register ('%')
                *trie_data_ptr = trie_next(*trie_data_ptr, c);
                assert(*trie_data_ptr != NULL);
                return OPERAND_REG;
            }
            else if (c == '(' || '0' <= c && c <= '9')
            {
                // memory
            }
            return FAILED_TRANSFER;
        case OPERAND_IMM:
                // immediate number
                // use string2uint_next as DFA to parse immediate number
                *num_state = string2uint_next(*num_state, c, &(od->imm));
            return;
        default:
            break;
    }
}


void parse_instruction(const char *str, inst_t *inst)
{
    char op_str[64] = {'\0'};
    int op_len = 0;
    char src_str[64] = {'\0'};
    int src_len = 0;
    char dst_str[64] = {'\0'};
    int dst_len = 0;

    char c;
    int count_parentheses = 0;
    int state = 0;

    for (int i = 0; i < strlen(str); ++ i)
    {
        c = str[i];
        if (c == '(' || c == ')')
        {
            count_parentheses ++;
        }

        if (state == 0 && c != ' ')
        {
            state = 1;
        }
        else if (state == 1 && c == ' ')
        {
            state = 2;
            continue;
        }
        else if (state == 2 && c != ' ')
        {
            state = 3;
        }
        else if (state == 3 && c == ',' && (count_parentheses == 0 || count_parentheses == 2))
        {
            state = 4;
            continue;
        }
        else if (state == 4 && c != ' ' && c != ',')
        {
            state = 5;
        }
        else if (state == 5 && c == ' ')
        {
            state = 6;
            continue;
        }

        if (state == 1)
        {
            op_str[op_len] = c;
            op_len ++;
            continue;
        }
        else if (state == 3)
        {
            src_str[src_len] = c;
            src_len ++;
            continue;
        }
        else if (state == 5)
        {
            dst_str[dst_len] = c;
            dst_len ++;
            continue;
        }
    }

    // op_str, src_str, dst_str
    // strlen(str)
    parse_operand(src_str, &(inst->src));
    parse_operand(dst_str, &(inst->dst));

    inst->op = (op_t)try_get_from_trie(&operator_mapping, op_str);

#ifdef DEBUG_PARSEINST
    printf("[%s (%d)] [%s (%d)] [%s (%d)]\n", op_str, inst->op, src_str, inst->src.type, dst_str, inst->dst.type);
#endif
}

// parse the string assembly operand to od_t instance
void parse_operand(const char *str, od_t *od)
{
    // str: the stripped compact operand string: turned to lower cases before parsing
    // od: the data structure to store operand
    od->type = OD_EMPTY;
    od->imm = 0;
    od->scal = 0;
    od->reg1 = 0;
    od->reg2 = 0;

    int str_len = strlen(str);
    if (str_len == 0)
    {
        // empty operand
        return;
    }    

    // parse the operand string
    if (str[0] == '$')
    {
        // immediate number
        od->type = OD_IMM;
        // try to parse the immediate number 64
        // condition short cut would not bring extra burden
        od->imm = string2uint_range(str, 1, -1);
    }
    else if (str[0] == '%')
    {
        // register
        od->type = OD_REG;
        // match the correct register name
        od->reg1 = try_get_from_trie(&register_mapping, (char *)str);
        return;
    }
    else
    {
        // should be a memory format, but check it
        // split imm(reg1,reg2,scal)
        char imm[64] = {'\0'};
        int imm_len = 0;
        char reg1[8] = {'\0'};
        int reg1_len = 0;
        char reg2[8] = {'\0'};
        int reg2_len = 0;
        char scal[2] = {'\0'};
        int scal_len = 0;

        int count_parentheses = 0;
        int count_comma = 0;
        // scan
        for (int i = 0; i < str_len; ++ i)
        {
            if (str[i] == '(' || str[i] == ')')
            {
                count_parentheses ++;
                continue;
            }            
            else if (str[i] == ',')
            {
                count_comma ++;
                continue;
            }
            else
            {
                if (count_parentheses == 0)
                {
                    // imm
                    imm[imm_len] = str[i];
                    imm_len ++;
                }
                else if (count_parentheses == 1)
                {
                    if (count_comma == 0)
                    {
                        // ...(reg1
                        reg1[reg1_len] = str[i];
                        reg1_len ++;
                    }
                    else if (count_comma == 1)
                    {
                        // ...(...,reg2
                        reg2[reg2_len] = str[i];
                        reg2_len ++;
                    }
                    else if (count_comma == 2)
                    {
                        // ...(...,...,scal
                        scal[scal_len] = str[i];
                        scal_len ++;
                    }
                }
            }
        }

        // parse imm
        if (imm_len > 0)
        {
            od->imm = string2uint(imm);
            if (count_parentheses == 0)
            {
                // imm
                od->type = OD_MEM_IMM;
                return;
            }
        }
        // parse scale
        if (scal_len > 0)
        {
            od->scal = string2uint(scal);
            if (od->scal != 1 && od->scal != 2 && od->scal != 4 && od->scal != 8)
            {
#ifdef DEBUG_PARSEINST
                printf("parse operand %s\n    scale number %s must be 1,2,4,8\n", str, scal);
#endif
                exit(0);
            }
        }
        // parse reg1
        if (reg1_len > 0)
        {
            od->reg1 = try_get_from_trie(&register_mapping, reg1);
        }
        // parse reg2
        if (reg2_len > 0)
        {
            od->reg2 = try_get_from_trie(&register_mapping, reg2);
        }

        // set types
        if (count_comma == 0)
        {
            // (r)
            od->type = OD_MEM_REG1;
        }
        else if (count_comma == 1)
        {
            // (r,r)
            od->type = OD_MEM_REG1_REG2;
        }
        else if (count_comma == 2)
        {
            if (reg1_len == 0)
            {
                // (,r,s)
                od->type = OD_MEM_REG2_SCAL;
            }
            else
            {
                // (r,r,s)
                od->type = OD_MEM_REG1_REG2_SCAL;
            }
        }
        // bias 1 for MEM_IMM_[.*]
        if (imm_len > 0)
        od->type ++;
    }
}