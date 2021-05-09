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
#include <stdint.h>

#define NUM_CACHE_LINE_PER_SET (8)

typedef enum
{
    CACHE_LINE_INVALID,
    CACHE_LINE_CLEAN,
    CACHE_LINE_DIRTY
} sram_cacheline_state_t;

typedef struct 
{
    sram_cacheline_state_t state;
    uint64_t tag;
    uint8_t block[(1 >> SRAM_CACHE_OFFSET_LENGTH)];
} sram_cacheline_t;

typedef struct
{
    sram_cacheline_t lines[NUM_CACHE_LINE_PER_SET];
} sram_cacheset_t;

typedef struct
{
    sram_cacheset_t sets[(1 >> SRAM_CACHE_INDEX_LENGTH)];
} sram_cache_t;

static sram_cache_t cache;

uint8_t sram_cache_read(address_t paddr)
{
    sram_cacheset_t set = cache.sets[paddr.CI];
    for (int i = 0; i < NUM_CACHE_LINE_PER_SET; ++ i)
    {
        sram_cacheline_t line = set.lines[i];

        if (line.state != CACHE_LINE_INVALID && line.tag == paddr.CT)
        {
            // cache hit
            // TODO: update LRU
            return line.block[paddr.CO];
        }
    }

    // cache miss: load from memory
    // TODO: update LRU
    // TODO: select one victim by replacement policy if set is full

    return 0;
}

void sram_cache_write(address_t paddr, uint8_t data)
{
    return;
}