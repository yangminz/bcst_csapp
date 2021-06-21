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
static void parse_instruction(const char *str, inst_t *inst);
static void parse_operand(const char *str, od_t *od);
static uint64_t compute_operand(od_t *od);

// interpret the operand
static uint64_t compute_operand(od_t *od)
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

static void parse_instruction(const char *str, inst_t *inst)
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

    debug_printf(DEBUG_PARSEINST, "[%s (%d)] [%s (%d)] [%s (%d)]\n", op_str, inst->op, src_str, inst->src.type, dst_str, inst->dst.type);
}

// parse the string assembly operand to od_t instance
static void parse_operand(const char *str, od_t *od)
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
                debug_printf(DEBUG_PARSEINST, "parse operand %s\n    scale number %s must be 1,2,4,8\n", scal);
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

/*======================================*/
/*      instruction handlers            */
/*======================================*/

// insturction (sub)set
// In this simulator, the instructions have been decoded and fetched
// so there will be no page fault during fetching
// otherwise the instructions must handle the page fault (swap in from disk) first
// and then re-fetch the instruction and do decoding
// and finally re-run the instruction

static void mov_handler             (od_t *src_od, od_t *dst_od);
static void push_handler            (od_t *src_od, od_t *dst_od);
static void pop_handler             (od_t *src_od, od_t *dst_od);
static void leave_handler           (od_t *src_od, od_t *dst_od);
static void call_handler            (od_t *src_od, od_t *dst_od);
static void ret_handler             (od_t *src_od, od_t *dst_od);
static void add_handler             (od_t *src_od, od_t *dst_od);
static void sub_handler             (od_t *src_od, od_t *dst_od);
static void cmp_handler             (od_t *src_od, od_t *dst_od);
static void jne_handler             (od_t *src_od, od_t *dst_od);
static void jmp_handler             (od_t *src_od, od_t *dst_od);

// handler table storing the handlers to different instruction types
typedef void (*handler_t)(od_t *, od_t *);
// look-up table of pointers to function
static handler_t handler_table[NUM_INSTRTYPE] = {
    &mov_handler,               // 0
    &push_handler,              // 1
    &pop_handler,               // 2
    &leave_handler,             // 3
    &call_handler,              // 4
    &ret_handler,               // 5
    &add_handler,               // 6
    &sub_handler,               // 7
    &cmp_handler,               // 8
    &jne_handler,               // 9
    &jmp_handler,               // 10
};

// update the rip pointer to the next instruction sequentially
static inline void increase_pc()
{
    // we are handling the fixed-length of assembly string here
    // but their size can be variable as true X86 instructions
    // that's because the operands' sizes follow the specific encoding rule
    // the risc-v is a fixed length ISA
    cpu_pc.rip = cpu_pc.rip + sizeof(char) * MAX_INSTRUCTION_CHAR;
}

// instruction handlers

static void mov_handler(od_t *src_od, od_t *dst_od)
{
    uint64_t src = compute_operand(src_od);
    uint64_t dst = compute_operand(dst_od);

    if (src_od->type == OD_REG && dst_od->type == OD_REG)
    {
        // src: register
        // dst: register
        *(uint64_t *)dst = *(uint64_t *)src;
        increase_pc();
        cpu_flags.__flags_value = 0;
        return;
    }
    else if (src_od->type == OD_REG && dst_od->type >= OD_MEM_IMM)
    {
        // src: register
        // dst: virtual address
        cpu_write64bits_dram(
            va2pa(dst), 
            *(uint64_t *)src);
        increase_pc();
        cpu_flags.__flags_value = 0;
        return;
    }
    else if (src_od->type >= OD_MEM_IMM && dst_od->type == OD_REG)
    {
        // src: virtual address
        // dst: register
        *(uint64_t *)dst = cpu_read64bits_dram(va2pa(src));
        increase_pc();
        cpu_flags.__flags_value = 0;
        return;
    }
    else if (src_od->type == OD_IMM && dst_od->type == OD_REG)
    {
        // src: immediate number (uint64_t bit map)
        // dst: register
        *(uint64_t *)dst = src;
        increase_pc();
        cpu_flags.__flags_value = 0;
        return;
    }
}

static void push_handler(od_t *src_od, od_t *dst_od)
{
    uint64_t src = compute_operand(src_od);
    // uint64_t dst = compute_operand(dst_od);

    if (src_od->type == OD_REG)
    {
        // src: register
        // dst: empty
        cpu_reg.rsp = cpu_reg.rsp - 8;
        cpu_write64bits_dram(
            va2pa(cpu_reg.rsp), 
            *(uint64_t *)src);
        increase_pc();
        cpu_flags.__flags_value = 0;
        return;
    }
}

static void pop_handler(od_t *src_od, od_t *dst_od)
{
    uint64_t src = compute_operand(src_od);
    // uint64_t dst = compute_operand(dst_od);

    if (src_od->type == OD_REG)
    {
        // src: register
        // dst: empty
        uint64_t old_val = cpu_read64bits_dram(
            va2pa(cpu_reg.rsp));
        cpu_reg.rsp = cpu_reg.rsp + 8;
        *(uint64_t *)src = old_val;
        increase_pc();
        cpu_flags.__flags_value = 0;
        return;
    }
}

static void leave_handler(od_t *src_od, od_t *dst_od)
{
    // movq %rbp, %rsp
    cpu_reg.rsp = cpu_reg.rbp;

    // popq %rbp
    uint64_t old_val = cpu_read64bits_dram(
        va2pa(cpu_reg.rsp));
    cpu_reg.rsp = cpu_reg.rsp + 8;
    cpu_reg.rbp = old_val;
    increase_pc();
    cpu_flags.__flags_value = 0;
}

static void call_handler(od_t *src_od, od_t *dst_od)
{
    uint64_t src = compute_operand(src_od);
    // uint64_t dst = compute_operand(dst_od);

    // src: immediate number: virtual address of target function starting
    // dst: empty
    // push the return value
    cpu_reg.rsp = cpu_reg.rsp - 8;
    cpu_write64bits_dram(
        va2pa(cpu_reg.rsp),
        cpu_pc.rip + sizeof(char) * MAX_INSTRUCTION_CHAR);
    // jump to target function address
    // TODO: support PC relative addressing
    cpu_pc.rip = src;
    cpu_flags.__flags_value = 0;
}

static void ret_handler(od_t *src_od, od_t *dst_od)
{
    // uint64_t src = compute_operand(src_od);
    // uint64_t dst = compute_operand(dst_od);

    // src: empty
    // dst: empty
    // pop rsp
    uint64_t ret_addr = cpu_read64bits_dram(
        va2pa(cpu_reg.rsp));
    cpu_reg.rsp = cpu_reg.rsp + 8;
    // jump to return address
    cpu_pc.rip = ret_addr;
    cpu_flags.__flags_value = 0;
}

static void add_handler(od_t *src_od, od_t *dst_od)
{
    uint64_t src = compute_operand(src_od);
    uint64_t dst = compute_operand(dst_od);

    if (src_od->type == OD_REG && dst_od->type == OD_REG)
    {
        // src: register (value: int64_t bit map)
        // dst: register (value: int64_t bit map)
        uint64_t val = *(uint64_t *)dst + *(uint64_t *)src;

        int val_sign = ((val >> 63) & 0x1);
        int src_sign = ((*(uint64_t *)src >> 63) & 0x1);
        int dst_sign = ((*(uint64_t *)dst >> 63) & 0x1);

        // set condition flags
        cpu_flags.CF = (val < *(uint64_t *)src); // unsigned
        cpu_flags.ZF = (val == 0);
        cpu_flags.SF = val_sign;
        cpu_flags.OF = (src_sign == 0 && dst_sign == 0 && val_sign == 1) || (src_sign == 1 && dst_sign == 1 && val_sign == 0);

        // update registers
        *(uint64_t *)dst = val;
        // signed and unsigned value follow the same addition. e.g.
        // 5 = 0000000000000101, 3 = 0000000000000011, -3 = 1111111111111101, 5 + (-3) = 0000000000000010
        increase_pc();
        return;
    }
}

static void sub_handler(od_t *src_od, od_t *dst_od)
{
    uint64_t src = compute_operand(src_od);
    uint64_t dst = compute_operand(dst_od);

    if (src_od->type == OD_IMM && dst_od->type == OD_REG)
    {
        // src: register (value: int64_t bit map)
        // dst: register (value: int64_t bit map)
        // dst = dst - src = dst + (-src)
        uint64_t val = *(uint64_t *)dst + (~src + 1);

        int val_sign = ((val >> 63) & 0x1);
        int src_sign = ((src >> 63) & 0x1);
        int dst_sign = ((*(uint64_t *)dst >> 63) & 0x1);

        // set condition flags
        cpu_flags.CF = (val > *(uint64_t *)dst); // unsigned

        cpu_flags.ZF = (val == 0);
        cpu_flags.SF = val_sign;

        cpu_flags.OF = (src_sign == 1 && dst_sign == 0 && val_sign == 1) || (src_sign == 0 && dst_sign == 1 && val_sign == 0);

        // update registers
        *(uint64_t *)dst = val;
        // signed and unsigned value follow the same addition. e.g.
        // 5 = 0000000000000101, 3 = 0000000000000011, -3 = 1111111111111101, 5 + (-3) = 0000000000000010
        increase_pc();
        return;
    }
}

static void cmp_handler(od_t *src_od, od_t *dst_od)
{
    uint64_t src = compute_operand(src_od);
    uint64_t dst = compute_operand(dst_od);

    if (src_od->type == OD_IMM && dst_od->type >= OD_MEM_IMM)
    {
        // src: register (value: int64_t bit map)
        // dst: register (value: int64_t bit map)
        // dst = dst - src = dst + (-src)
        uint64_t dval = cpu_read64bits_dram(va2pa(dst));
        uint64_t val = dval + (~src + 1);

        int val_sign = ((val >> 63) & 0x1);
        int src_sign = ((src >> 63) & 0x1);
        int dst_sign = ((dval >> 63) & 0x1);

        // set condition flags
        cpu_flags.CF = (val > dval); // unsigned

        cpu_flags.ZF = (val == 0);
        cpu_flags.SF = val_sign;

        cpu_flags.OF = (src_sign == 1 && dst_sign == 0 && val_sign == 1) || (src_sign == 0 && dst_sign == 1 && val_sign == 0);

        // signed and unsigned value follow the same addition. e.g.
        // 5 = 0000000000000101, 3 = 0000000000000011, -3 = 1111111111111101, 5 + (-3) = 0000000000000010
        increase_pc();
        return;
    }
}

static void jne_handler(od_t *src_od, od_t *dst_od)
{
    uint64_t src = compute_operand(src_od);

    // src_od is actually a instruction memory address
    // but we are interpreting it as an immediate number
    if (cpu_flags.ZF == 0)
    {
        // last instruction value != 0
        cpu_pc.rip = src;
    }
    else
    {
        // last instruction value == 0
        increase_pc();
    }
    cpu_flags.__flags_value = 0;
}

static void jmp_handler(od_t *src_od, od_t *dst_od)
{
    uint64_t src = compute_operand(src_od);
    cpu_pc.rip = src;
    cpu_flags.__flags_value = 0;
}

// instruction cycle is implemented in CPU
// the only exposed interface outside CPU
void instruction_cycle()
{
    // FETCH: get the instruction string by program counter
    char inst_str[MAX_INSTRUCTION_CHAR + 10];
    cpu_readinst_dram(va2pa(cpu_pc.rip), inst_str);

#ifdef DEBUG_INSTRUCTION_CYCLE
    printf("%8lx    %s\n", cpu_pc.rip, inst_str);
#endif

    // DECODE: decode the run-time instruction operands
    inst_t inst;
    parse_instruction(inst_str, &inst);

    // EXECUTE: get the function pointer or handler by the operator
    handler_t handler = handler_table[inst.op];
    // update CPU and memory according the instruction
    handler(&(inst.src), &(inst.dst));
}

#ifdef DEBUG_PARSE_INSTRUCTION

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

#endif

#ifdef DEBUG_INSTRUCTION_CYCLE

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
    TestAddFunctionCallAndComputation();
    TestSumRecursiveCondition();

    finally_cleanup();
    return 0;
}

#endif