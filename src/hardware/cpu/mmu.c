// Memory Management Unit 
#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<headers/cpu.h>
#include<headers/memory.h>
#include<headers/common.h>

uint64_t va2pa(uint64_t vaddr, core_t *cr)
{
    return vaddr % PHYSICAL_MEMORY_SPACE;
}