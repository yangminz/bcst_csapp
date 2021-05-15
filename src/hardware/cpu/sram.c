/* BCST - Introduction to Computer Systems
 * Author:      yangminz@outlook.com
 * Github:      https://github.com/yangminz/bcst_csapp
 * Bilibili:    https://space.bilibili.com/4564101
 * Zhihu:       https://www.zhihu.com/people/zhao-yang-min
 * This project (code repository and videos) is exclusively owned by yangminz 
 * and shall not be used for commercial and profitting purpose 
 * without yangminz's permission.
 */

#include "headers/address.h"
#include "headers/memory.h"
#include <stdint.h>
#include <assert.h>

#define NUM_CACHE_LINE_PER_SET (8)

// write-back and write-allocate
typedef enum
{
    CACHE_LINE_INVALID,
    CACHE_LINE_CLEAN,
    CACHE_LINE_DIRTY
} sram_cacheline_state_t;

typedef struct 
{
    sram_cacheline_state_t state;
    int time;  // timer to find LRU line inside one set
    uint64_t tag;
    uint8_t block[(1 << SRAM_CACHE_OFFSET_LENGTH)];
} sram_cacheline_t;

typedef struct
{
    sram_cacheline_t lines[NUM_CACHE_LINE_PER_SET];
} sram_cacheset_t;

typedef struct
{
    sram_cacheset_t sets[(1 << SRAM_CACHE_INDEX_LENGTH)];
} sram_cache_t;

static sram_cache_t cache;

uint8_t sram_cache_read(uint64_t paddr_value)
{
    address_t paddr = {
        .paddr_value = paddr_value,
    };
    
    sram_cacheset_t set = cache.sets[paddr.CI];

    // update LRU time
    sram_cacheline_t *victim = NULL;
    sram_cacheline_t *invalid = NULL;
    int max_time = -1;

    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; ++ i)
    {
        set.lines[i].time ++;

        if (max_time < set.lines[i].time)
        {
            // select this line as victim by LRU policy
            // replace it when all lines are valid
            victim = &(set.lines[i]);
            max_time = set.lines[i].time;
        }

        if (set.lines[i].state == CACHE_LINE_INVALID)
        {
            // exist one invalid line as candidate for cache miss
            invalid = &(set.lines[i]);
        }
    }

    // try cache hit
    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; ++ i)
    {
        sram_cacheline_t line = set.lines[i];

        if (line.state != CACHE_LINE_INVALID && line.tag == paddr.CT)
        {
            // cache hit
            // update LRU time
            line.time = 0;

            // find the byte
            return line.block[paddr.CO];
        }
    }

    // cache miss: load from memory

    // try to find one free cache line
    if (invalid != NULL)
    {
        // load data from DRAM to this invalid cache line
        bus_read_cacheline(paddr.paddr_value, &(invalid->block));

        // update cache line state
        invalid->state = CACHE_LINE_CLEAN;

        // update LRU
        invalid->time = 0;

        // update tag
        invalid->tag = paddr.CT;

        return invalid->block[paddr.CO];
    }

    // no free cache line, use LRU policy
    assert(victim != NULL);

    if (victim->state == CACHE_LINE_DIRTY)
    {
        // write back the dirty line to dram
        bus_write_cacheline(paddr.paddr_value, victim);
    }
    // if CACHE_LINE_CLEAN discard this victim directly
    // update state
    victim->state = CACHE_LINE_INVALID;

    // read from dram
    // load data from DRAM to this invalid cache line
    bus_read_cacheline(paddr.paddr_value, &(victim->block));

    // update cache line state
    victim->state = CACHE_LINE_CLEAN;

    // update LRU
    victim->time = 0;

    // update tag
    victim->tag = paddr.CT;

    return victim->block[paddr.CO];
}

void sram_cache_write(uint64_t paddr_value, uint8_t data)
{
    address_t paddr = {
        .paddr_value = paddr_value,
    };

    sram_cacheset_t set = cache.sets[paddr.CI];

    // update LRU time
    sram_cacheline_t *victim = NULL;
    sram_cacheline_t *invalid = NULL;   // for write-allocate
    int max_time = -1;

    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; ++ i)
    {
        set.lines[i].time ++;

        if (max_time < set.lines[i].time)
        {
            // select this line as victim by LRU policy
            // replace it when all lines are valid
            victim = &(set.lines[i]);
            max_time = set.lines[i].time;
        }

        if (set.lines[i].state == CACHE_LINE_INVALID)
        {
            // exist one invalid line as candidate for cache miss
            invalid = &(set.lines[i]);
        }
    }

    // try cache hit
    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; ++ i)
    {
        sram_cacheline_t line = set.lines[i];

        if (line.state != CACHE_LINE_INVALID && line.tag == paddr.CT)
        {
            // cache hit

            // update LRU time
            line.time = 0;
            
            // find the byte
            line.block[paddr.CO] = data;
            
            // update state
            line.state = CACHE_LINE_DIRTY;

            return;
        }
    }

    // cache miss: load from memory

    // write-allocate

    // try to find one free cache line
    if (invalid != NULL)
    {
        // load data from DRAM to this invalid cache line
        bus_read_cacheline(paddr.paddr_value, &(invalid->block));

        // update cache line state
        invalid->state = CACHE_LINE_DIRTY;

        // update LRU
        invalid->time = 0;

        // update tag
        invalid->tag = paddr.CT;

        // write data
        invalid->block[paddr.CO] = data;

        return;
    }

    // no free cache line, use LRU policy
    assert(victim != NULL);

    if (victim->state == CACHE_LINE_DIRTY)
    {
        // write back the dirty line to dram
        bus_write_cacheline(paddr.paddr_value, victim);
    }
    // if CACHE_LINE_CLEAN discard this victim directly
    // update state
    victim->state = CACHE_LINE_INVALID;

    // read from dram
    // write-allocate
    // load data from DRAM to this invalid cache line
    bus_read_cacheline(paddr.paddr_value, &(victim->block));

    // update cache line state
    victim->state = CACHE_LINE_DIRTY;

    // update LRU
    victim->time = 0;

    // update tag
    victim->tag = paddr.CT;

    victim->block[paddr.CO] = data;
}

