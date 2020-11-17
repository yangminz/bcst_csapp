#include <cpu/register.h>
#include <cpu/mmu.h>
#include <memory/dram.h>
#include <stdio.h>

#define SRAM_CACHE_SETTING 0

// flighting

uint64_t read64bits_dram(uint64_t paddr)
{
    if (SRAM_CACHE_SETTING == 1)
    {
        return 0x0;
    }

    uint64_t val = 0x0;

    val += (((uint64_t)mm[paddr + 0 ]) << 0);
    val += (((uint64_t)mm[paddr + 1 ]) << 8);
    val += (((uint64_t)mm[paddr + 2 ]) << 16);
    val += (((uint64_t)mm[paddr + 3 ]) << 24);
    val += (((uint64_t)mm[paddr + 4 ]) << 32);
    val += (((uint64_t)mm[paddr + 5 ]) << 40);
    val += (((uint64_t)mm[paddr + 6 ]) << 48);
    val += (((uint64_t)mm[paddr + 7 ]) << 56);

    return val;
}

void write64bits_dram(uint64_t paddr, uint64_t data)
{
    if (SRAM_CACHE_SETTING == 1)
    {
        return;
    }

    mm[paddr + 0] = (data >> 0 ) & 0xff;
    mm[paddr + 1] = (data >> 8 ) & 0xff;
    mm[paddr + 2] = (data >> 16) & 0xff;
    mm[paddr + 3] = (data >> 24) & 0xff;
    mm[paddr + 4] = (data >> 32) & 0xff;
    mm[paddr + 5] = (data >> 40) & 0xff;
    mm[paddr + 6] = (data >> 48) & 0xff;
    mm[paddr + 7] = (data >> 56) & 0xff;
}

void print_register()
{
    printf("rax = %16lx\trbx = %16lx\trcx = %16lx\trdx = %16lx\n",
        reg.rax, reg.rbx, reg.rcx, reg.rdx);
    printf("rsi = %16lx\trdi = %16lx\trbp = %16lx\trsp = %16lx\n",
        reg.rsi, reg.rdi, reg.rbp, reg.rsp);
    printf("rip = %16lx\n", reg.rip);
}

void print_stack()
{
    int n = 10;

    uint64_t *high = (uint64_t*)&mm[va2pa(reg.rsp)];
    high = &high[n];

    uint64_t rsp_start = reg.rsp + n * 8;

    for (int i = 0; i < 2 * n; ++ i)
    {
        uint64_t *ptr = (uint64_t *)(high - i);
        printf("0x%016lx : %16lx", rsp_start, (uint64_t)*ptr);

        if (i == n)
        {
            printf(" <== rsp");
        }

        rsp_start = rsp_start - 8;

        printf("\n");
    }
}