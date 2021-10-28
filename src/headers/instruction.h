/* BCST - Introduction to Computer Systems
 * Author:      yangminz@outlook.com
 * Github:      https://github.com/yangminz/bcst_csapp
 * Bilibili:    https://space.bilibili.com/4564101
 * Zhihu:       https://www.zhihu.com/people/zhao-yang-min
 * This project (code repository and videos) is exclusively owned by yangminz 
 * and shall not be used for commercial and profitting purpose 
 * without yangminz's permission.
 */

// include guards to prevent double declaration of any identifiers 
// such as types, enums and static variables
#ifndef INSTRUCTION_GUARD
#define INSTRUCTION_GUARD

#include <stdint.h>

/*======================================*/
/*      instruction set architecture    */
/*======================================*/

typedef enum OPERAND_TYPE
{
    OD_EMPTY,                  // 0
    OD_IMM,                    // 1
    OD_REG,                    // 2
    OD_MEM,                    // 3
} od_type_t;

typedef struct OPERAND_STRUCT
{
    od_type_t   type;   // OD_IMM, OD_REG, OD_MEM
    uint64_t    value;  // the value
} od_t;

// handler table storing the handlers to different instruction types
typedef void (*op_t)(od_t *, od_t *);

// local variables are allocated in stack in run-time
// we don't consider local STATIC variables
// ref: Computer Systems: A Programmer's Perspective 3rd
// Chapter 7 Linking: 7.5 Symbols and Symbol Tables
typedef struct INST_STRUCT
{
    op_t    op;         // enum of operators. e.g. mov, call, etc.
    od_t    src;        // operand src of instruction
    od_t    dst;        // operand dst of instruction
} inst_t;

#define MAX_NUM_INSTRUCTION_CYCLE 100

#endif
