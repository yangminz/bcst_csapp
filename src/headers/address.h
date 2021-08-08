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
#ifndef ADDRESS_GUARD
#define ADDRESS_GUARD

#include <stdint.h>

#ifndef CACHE_SIMULATION_VERIFICATION
/*  for cache simulator verification
    use the marcos passed in
 */
#define SRAM_CACHE_INDEX_LENGTH (6)
#define SRAM_CACHE_OFFSET_LENGTH (6)
#define SRAM_CACHE_TAG_LENGTH (4)
#endif

#define PHYSICAL_PAGE_OFFSET_LENGTH (12)
#define PHYSICAL_PAGE_NUMBER_LENGTH (4)
#define PHYSICAL_ADDRESS_LENGTH (16)

#define VIRTUAL_PAGE_OFFSET_LENGTH (12)
#define VIRTUAL_PAGE_NUMBER_LENGTH (9)  // 9 + 9 + 9 + 9 = 36
#define VIRTUAL_ADDRESS_LENGTH (48)

#define TLB_CACHE_OFFSET_LENGTH (12)
#define TLB_CACHE_INDEX_LENGTH (4)
#define TLB_CACHE_TAG_LENGTH (32)

/*
+--------+--------+--------+--------+---------------+
|  VPN1  |  VPN2  |  VPN3  |  VPN4  |               |
+--------+--------+--------+-+------+      VPO      |
|    TLBT                    | TLBI |               |
+---------------+------------+------+---------------+
                |        PPN        |      PPO      |
                +-------------------+--------+------+
                |        CT         |   CI   |  CO  |
                +-------------------+--------+------+
*/
typedef union
{
    uint64_t address_value;

    // physical address: 52    
    struct
    {
        union
        {
            uint64_t paddr_value : PHYSICAL_ADDRESS_LENGTH;
            struct
            {
                uint64_t ppo : PHYSICAL_PAGE_OFFSET_LENGTH;
                uint64_t ppn : PHYSICAL_PAGE_NUMBER_LENGTH;
            };
        };
    };

    // sram cache: 52
    struct
    {
        uint64_t co : SRAM_CACHE_OFFSET_LENGTH;
        uint64_t ci : SRAM_CACHE_INDEX_LENGTH;
        uint64_t ct : SRAM_CACHE_TAG_LENGTH;
    };

    // virtual address: 48    
    struct
    {
        union
        {
            uint64_t vaddr_value : VIRTUAL_ADDRESS_LENGTH;
            struct
            {
                uint64_t vpo  : VIRTUAL_PAGE_OFFSET_LENGTH;
                uint64_t vpn4 : VIRTUAL_PAGE_NUMBER_LENGTH;
                uint64_t vpn3 : VIRTUAL_PAGE_NUMBER_LENGTH;
                uint64_t vpn2 : VIRTUAL_PAGE_NUMBER_LENGTH;
                uint64_t vpn1 : VIRTUAL_PAGE_NUMBER_LENGTH;
            };
        };
    };

    // TLB cache: 48
    struct
    {
        uint64_t tlbo : TLB_CACHE_OFFSET_LENGTH;   // virtual page offset
        uint64_t tlbi : TLB_CACHE_INDEX_LENGTH;    // TLB set index
        uint64_t tlbt : TLB_CACHE_TAG_LENGTH;      // TLB line tag
    };
} address_t;

#endif
