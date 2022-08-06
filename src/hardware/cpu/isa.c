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

/*  A Message from Interrupt & Page Fault:
 *      For all instructions involving address translation (MMU, i.e. func `va2pa`),
 *      `va2pa` must be called BEFORE rip increment `increase_pc`.
 *      Because address translation may fail, page fault may happen, resulting 
 *      page fault handling/interrupt and context switch.
 * 
 *      Happy path: No page fault
 *          everything is good.
 * 
 *      Sad path: Page faults
 *          then `va2pa` will not return to current process (process 1).
 *          RIP pushed to trapframe will be the current RIP.
 *          So when the execution of process 1 is resumed, the return
 *          instruction is just the faulting instruction.
 *          Which means the faulting `mov` should be executed twice:
 * 
 *          process 1, first `mov`: page fault
 *              process 2
 *              process 3
 *          process 1, second `mov`: no page fault
 */

void mov_handler(od_t *src_od, od_t *dst_od)
{
    if (src_od->type == OD_REG && dst_od->type == OD_REG)
    {
        // src: register
        // dst: register
        DEREF_VALUE(dst_od) = DEREF_VALUE(src_od);
        increase_pc();
        cpu_flags.__flags_value = 0;
        return;
    }
    else if (src_od->type == OD_REG && dst_od->type == OD_MEM)
    {
        // src: register
        // dst: virtual address
        virtual_write_data(
            dst_od->value,
            DEREF_VALUE(src_od));
        increase_pc();
        cpu_flags.__flags_value = 0;
        return;
    }
    else if (src_od->type == OD_MEM && dst_od->type == OD_REG)
    {
        // src: virtual address
        // dst: register
        DEREF_VALUE(dst_od) = virtual_read_data(src_od->value);
        increase_pc();
        cpu_flags.__flags_value = 0;
        return;
    }
    else if (src_od->type == OD_IMM && dst_od->type == OD_REG)
    {
        // src: immediate number (uint64_t bit map)
        // dst: register
        DEREF_VALUE(dst_od) = (src_od->value);
        increase_pc();
        cpu_flags.__flags_value = 0;
        return;
    }
}

void push_handler(od_t *src_od, od_t *dst_od)
{
    if (src_od->type == OD_REG)
    {
        // src: register
        // dst: empty
        cpu_reg.rsp = cpu_reg.rsp - 8;
        virtual_write_data(
            cpu_reg.rsp, 
            DEREF_VALUE(src_od));
        increase_pc();
        cpu_flags.__flags_value = 0;
        return;
    }
}

void pop_handler(od_t *src_od, od_t *dst_od)
{
    if (src_od->type == OD_REG)
    {
        // src: register
        // dst: empty
        uint64_t old_val = virtual_read_data(cpu_reg.rsp);
        cpu_reg.rsp = cpu_reg.rsp + 8;
        DEREF_VALUE(src_od) = old_val;
        increase_pc();
        cpu_flags.__flags_value = 0;
        return;
    }
}

void leave_handler(od_t *src_od, od_t *dst_od)
{
    // movq %rbp, %rsp
    cpu_reg.rsp = cpu_reg.rbp;

    // popq %rbp
    uint64_t old_val = virtual_read_data(cpu_reg.rsp);
    cpu_reg.rsp = cpu_reg.rsp + 8;
    cpu_reg.rbp = old_val;
    increase_pc();
    cpu_flags.__flags_value = 0;
}

void call_handler(od_t *src_od, od_t *dst_od)
{
    // src: immediate number: virtual address of target function starting
    // dst: empty
    // push the return value
    cpu_reg.rsp = cpu_reg.rsp - 8;
    virtual_write_data(
        cpu_reg.rsp,
        cpu_pc.rip + sizeof(char) * MAX_INSTRUCTION_CHAR);
    // jump to target function address
    // TODO: support PC relative addressing
    cpu_pc.rip = (src_od->value);
    cpu_flags.__flags_value = 0;
}

void ret_handler(od_t *src_od, od_t *dst_od)
{
    // src: empty
    // dst: empty
    // pop rsp
    uint64_t ret_addr = virtual_read_data(cpu_reg.rsp);
    cpu_reg.rsp = cpu_reg.rsp + 8;
    // jump to return address
    cpu_pc.rip = ret_addr;
    cpu_flags.__flags_value = 0;
}

void add_handler(od_t *src_od, od_t *dst_od)
{
    if (src_od->type == OD_REG && dst_od->type == OD_REG)
    {
        // src: register (value: int64_t bit map)
        // dst: register (value: int64_t bit map)
        uint64_t val = DEREF_VALUE(dst_od) + DEREF_VALUE(src_od);

        int val_sign = ((val >> 63) & 0x1);
        int src_sign = ((DEREF_VALUE(src_od) >> 63) & 0x1);
        int dst_sign = ((DEREF_VALUE(dst_od) >> 63) & 0x1);

        // set condition flags
        cpu_flags.CF = (val < DEREF_VALUE(src_od)); // unsigned
        cpu_flags.ZF = (val == 0);
        cpu_flags.SF = val_sign;
        cpu_flags.OF = (src_sign == 0 && dst_sign == 0 && val_sign == 1) || (src_sign == 1 && dst_sign == 1 && val_sign == 0);

        // update registers
        DEREF_VALUE(dst_od) = val;
        // signed and unsigned value follow the same addition. e.g.
        // 5 = 0000000000000101, 3 = 0000000000000011, -3 = 1111111111111101, 5 + (-3) = 0000000000000010
        increase_pc();
        return;
    }
}

void sub_handler(od_t *src_od, od_t *dst_od)
{
    if (src_od->type == OD_IMM && dst_od->type == OD_REG)
    {
        // src: register (value: int64_t bit map)
        // dst: register (value: int64_t bit map)
        // (dst_od->value) = (dst_od->value) - (src_od->value) = (dst_od->value) + (-(src_od->value))
        uint64_t val = DEREF_VALUE(dst_od) + (~(src_od->value) + 1);

        int val_sign = ((val >> 63) & 0x1);
        int src_sign = (((src_od->value) >> 63) & 0x1);
        int dst_sign = ((DEREF_VALUE(dst_od) >> 63) & 0x1);

        // set condition flags
        cpu_flags.CF = (val > DEREF_VALUE(dst_od)); // unsigned

        cpu_flags.ZF = (val == 0);
        cpu_flags.SF = val_sign;

        cpu_flags.OF = (src_sign == 1 && dst_sign == 0 && val_sign == 1) || (src_sign == 0 && dst_sign == 1 && val_sign == 0);

        // update registers
        DEREF_VALUE(dst_od) = val;
        // signed and unsigned value follow the same addition. e.g.
        // 5 = 0000000000000101, 3 = 0000000000000011, -3 = 1111111111111101, 5 + (-3) = 0000000000000010
        increase_pc();
        return;
    }
}

void cmp_handler(od_t *src_od, od_t *dst_od)
{
    uint64_t val, dval;
    if (src_od->type == OD_IMM && dst_od->type == OD_MEM)
    {
        // src: register (value: int64_t bit map)
        // dst: register (value: int64_t bit map)
        // (dst_od->value) = (dst_od->value) - (src_od->value) = (dst_od->value) + (-(src_od->value))
        dval = virtual_read_data(dst_od->value);
    }
    else if (src_od->type == OD_IMM && dst_od->type == OD_REG)
    {
        dval = DEREF_VALUE(dst_od);
    }
    
    val = dval + (~(src_od->value) + 1);

    int val_sign = ((val >> 63) & 0x1);
    int src_sign = (((src_od->value) >> 63) & 0x1);
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

void jne_handler(od_t *src_od, od_t *dst_od)
{
    // src_od is actually a instruction memory address
    // but we are interpreting it as an immediate number
    if (cpu_flags.ZF == 0)
    {
        // last instruction value != 0
        cpu_pc.rip = (src_od->value);
    }
    else
    {
        // last instruction value == 0
        increase_pc();
    }
    cpu_flags.__flags_value = 0;
}

void jmp_handler(od_t *src_od, od_t *dst_od)
{
    cpu_pc.rip = (src_od->value);
    cpu_flags.__flags_value = 0;
}

void lea_handler(od_t *src_od, od_t *dst_od)
{
    if (src_od->type == OD_MEM && dst_od->type == OD_REG)
    {
        // src: virtual address - The effective address computed from instruction
        // dst: register - The register to load the effective address
        DEREF_VALUE(dst_od) = src_od->value;
        increase_pc();
        cpu_flags.__flags_value = 0;
        return;
    }
}

void int_handler(od_t *src_od, od_t *dst_od)
{
    if (src_od->type == OD_IMM)
    {
        // src: interrupt vector

        // Be careful here. Think why we need to increase RIP before interrupt?
        // This `int` instruction is executed by process 1,
        // but interrupt will cause OS's scheduling to process 2.
        // So this `int_handler` will not return.
        // When the execution of process 1 resumed, the system call is finished.
        // We want to execute the next instruction, so RIP pushed to trap frame
        // must be the next instruction.
        increase_pc();
        cpu_flags.__flags_value = 0;

        // This function will not return.
        interrupt_stack_switching(src_od->value);
    }
}

void nop_handler(od_t *src_od, od_t *dst_od)
{
    increase_pc();
}

// from inst.c
void parse_instruction(char *inst_str, inst_t *inst);

// time, the craft of god
static uint64_t global_time = 0;
static uint64_t timer_period = 5000000;

// instruction cycle is implemented in CPU
// the only exposed interface outside CPU
void instruction_cycle()
{
    // this is the entry point of the re-execution of
    // interrupt return instruction.
    // When a new process is scheduled, the first instruction/
    // return instruction should start here, jumping out of the
    // call stack of old process.
    // This is especially useful for page fault handling.
    setjmp(USER_INSTRUCTION_ON_IRET);

    global_time += 1;

    // FETCH: get the instruction string by program counter
    char inst_str[MAX_INSTRUCTION_CHAR + 10];
    virtual_read_inst(cpu_pc.rip, inst_str);

#ifdef DEBUG_INSTRUCTION_CYCLE
    printf("[%4ld] %8lx    %s\n", global_time, cpu_pc.rip, inst_str);
#endif

    // DECODE: decode the run-time instruction operands
    inst_t inst;
    parse_instruction(inst_str, &inst);

    // EXECUTE: get the function pointer or handler by the operator
    // update CPU and memory according the instruction
    inst.op(&(inst.src), &(inst.dst));
    
    // check timer interrupt from APIC
    if ((global_time % timer_period) == 0)
    {
        interrupt_stack_switching(0x81);
    }
}
