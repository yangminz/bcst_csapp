// Dynamic Random Access Memory
#include<headers/cpu.h>
#include<headers/memory.h>
#include<headers/common.h>

/*  
Be careful with the x86-64 little endian integer encoding
e.g. write 0x00007fd357a02ae0 to cache, the memory lapping should be:
    e0 2a a0 57 d3 7f 00 00
*/

// memory accessing used in instructions
uint64_t read64bits_dram(uint64_t paddr, core_t *cr)
{
    if (DEBUG_ENABLE_SRAM_CACHE == 1)
    {
        // try to load uint64_t from SRAM cache
        // little-endian
    }
    else
    {
        // read from DRAM directly
        // little-endian
        uint64_t val = 0x0;

        val += (((uint64_t)pm[paddr + 0 ]) << 0);
        val += (((uint64_t)pm[paddr + 1 ]) << 8);
        val += (((uint64_t)pm[paddr + 2 ]) << 16);
        val += (((uint64_t)pm[paddr + 3 ]) << 24);
        val += (((uint64_t)pm[paddr + 4 ]) << 32);
        val += (((uint64_t)pm[paddr + 5 ]) << 40);
        val += (((uint64_t)pm[paddr + 6 ]) << 48);
        val += (((uint64_t)pm[paddr + 7 ]) << 56);

        return val;
    }
}

void write64bits_dram(uint64_t paddr, uint64_t data, core_t *cr)
{
    if (DEBUG_ENABLE_SRAM_CACHE == 1)
    {
        // try to write uint64_t to SRAM cache
        // little-endian
    }
    else
    {
        // write to DRAM directly
        // little-endian
        pm[paddr + 0] = (data >> 0 ) & 0xff;
        pm[paddr + 1] = (data >> 8 ) & 0xff;
        pm[paddr + 2] = (data >> 16) & 0xff;
        pm[paddr + 3] = (data >> 24) & 0xff;
        pm[paddr + 4] = (data >> 32) & 0xff;
        pm[paddr + 5] = (data >> 40) & 0xff;
        pm[paddr + 6] = (data >> 48) & 0xff;
        pm[paddr + 7] = (data >> 56) & 0xff;
    }
}