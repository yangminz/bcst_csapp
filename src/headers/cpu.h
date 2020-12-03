// include guards to prevent double declaration of any identifiers 
// such as types, enums and static variables
#ifndef CPU_GUARD
#define CPU_GUARD

#include<stdint.h>
#include<stdlib.h>

/*======================================*/
/*      registers                       */
/*======================================*/

// struct of registers in each core
// resource accessible to the core itself only

typedef struct REGISTER_STRUCT 
{
    // return value
    union 
    {
        uint64_t rax;
        uint32_t eax;
        uint16_t ax;
        struct 
        { 
            uint8_t al; 
            uint8_t ah; 
        };
    };

    // callee saved
    union 
    {
        uint64_t rbx;
        uint32_t ebx;
        uint16_t bx;
        struct 
        { 
            uint8_t bl;
            uint8_t bh;
        };
    };

    // 4th argument
    union 
    {
        uint64_t rcx;
        uint32_t ecx;
        uint16_t cx;
        struct 
        { 
            uint8_t cl;
            uint8_t ch;
        };
    };
    // 3th argument
    union 
    {
        uint64_t rdx;
        uint32_t edx;
        uint16_t dx;
        struct 
        { 
            uint8_t dl;
            uint8_t dh;
        };
    };
    // 2nd argument
    union 
    {
        uint64_t rsi;
        uint32_t esi;
        uint16_t si;
        struct 
        { 
            uint8_t sil;
            uint8_t sih;
        };
    };
    // 1st argument
    union 
    {
        uint64_t rdi;
        uint32_t edi;
        uint16_t di;
        struct 
        { 
            uint8_t dil;
            uint8_t dih;
        };
    };

    // callee saved frame pointer
    union 
    {
        uint64_t rbp;
        uint32_t ebp;
        uint16_t bp;
        struct 
        { 
            uint8_t bpl;
            uint8_t bph;
        };
    };
    // stack pointer
    union 
    {
        uint64_t rsp;
        uint32_t esp;
        uint16_t sp;
        struct 
        { 
            uint8_t spl;
            uint8_t sph;
        };
    };

    // 5th argument
    union 
    {
        uint64_t r8;
        uint32_t r8d;
        uint16_t r8w;
        uint8_t  r8b;
    };
    // 6th argument
    union 
    {
        uint64_t r9;
        uint32_t r9d;
        uint16_t r9w;
        uint8_t  r9b;
    };

    // caller saved
    union 
    {
        uint64_t r10;
        uint32_t r10d;
        uint16_t r10w;
        uint8_t  r10b;
    };
    // caller saved
    union 
    {
        uint64_t r11;
        uint32_t r11d;
        uint16_t r11w;
        uint8_t  r11b;
    };
    
    // callee saved
    union 
    {
        uint64_t r12;
        uint32_t r12d;
        uint16_t r12w;
        uint8_t  r12b;
    };
    // callee saved
    union 
    {
        uint64_t r13;
        uint32_t r13d;
        uint16_t r13w;
        uint8_t  r13b;
    };
    // callee saved
    union 
    {
        uint64_t r14;
        uint32_t r14d;
        uint16_t r14w;
        uint8_t  r14b;
    };
    // callee saved
    union 
    {
        uint64_t r15;
        uint32_t r15d;
        uint16_t r15w;
        uint8_t  r15b;
    };
} reg_t;

/*======================================*/
/*      cpu core                        */
/*======================================*/

typedef struct CORE_STRUCT
{
    // program counter or instruction pointer
    union 
    {
        uint64_t rip;
        uint32_t eip;
    };
    
    // condition code flags of most recent (latest) operation
    // condition codes will only be set by the following integer arithmetic instructions

    /* integer arithmetic instructions
        inc     increment 1
        dec     decrement 1
        neg     negate
        not     complement
        ----------------------------
        add     add
        sub     subtract
        imul    multiply
        xor     exclusive or
        or      or
        and     and
        ----------------------------
        sal     left shift
        shl     left shift (same as sal)
        sar     arithmetic right shift
        shr     logical right shift
    */
    
    /* comparison and test instructions
        cmp     compare
        test    test
    */

    // carry flag: detect overflow for unsigned operations
    uint32_t CF;
    // zero flag: result is zero
    uint32_t ZF;
    // sign flag: result is negative: highest bit
    uint32_t SF;
    // overflow flag: detect overflow for signed operations
    uint32_t OF;

    // register files
    reg_t       reg;
} core_t;

// define cpu core array to support core level parallelism
#define NUM_CORES 1
core_t cores[NUM_CORES];
// active core for current task
uint64_t ACTIVE_CORE;

#define MAX_INSTRUCTION_CHAR 64
#define NUM_INSTRTYPE 14

// CPU's instruction cycle: execution of instructions
void instruction_cycle(core_t *cr);

/*--------------------------------------*/
// place the functions here because they requires the core_t type

/*--------------------------------------*/
// mmu functions

// translate the virtual address to physical address in MMU
// each MMU is owned by each core
uint64_t va2pa(uint64_t vaddr, core_t *cr);

// end of include guard
#endif