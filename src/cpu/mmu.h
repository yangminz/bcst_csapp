#ifndef mmu_guard
#define mmu_guard

// memory management unit
#include<stdint.h>

uint64_t va2pa(uint64_t vaddr);

#endif