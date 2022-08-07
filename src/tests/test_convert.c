/* BCST - Introduction to Computer Systems
 * Author:      yangminz@outlook.com
 * Github:      https://github.com/yangminz/bcst_csapp
 * Bilibili:    https://space.bilibili.com/4564101
 * Zhihu:       https://www.zhihu.com/people/zhao-yang-min
 * This project (code repository and videos) is exclusively owned by yangminz 
 * and shall not be used for commercial and profitting purpose 
 * without yangminz's permission.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "headers/common.h"
#include "headers/color.h"

static void test_float()
{
    printf("Testing converting uint32 to float ...\n");

    for (uint32_t i = 0; i < (1 << 30); ++ i)
    {
        uint32_t float_map = uint2float(i);
        float float_val = (float)i;
        assert(*(uint32_t *)&float_val == float_map);
    }

    printf(GREENSTR("Pass\n"));
}

static void test_string()
{
    printf("Testing converting string to uint64 ...\n");

    srand(123456);
    char pos_dec[64];
    char neg_dec[64];
    char pos_hex[64];
    char neg_hex[64];
    
    for (int i = 0; i < 10000000; ++ i)
    {
        uint64_t v = rand();

        sprintf(pos_dec, "%lu", v);
        sprintf(pos_hex, "0x%lx", v);
        sprintf(neg_dec, "-%lu", v);
        sprintf(neg_hex, "-0x%lx", v);

        assert(string2uint(pos_dec) == v);
        assert(string2uint(pos_hex) == v);
        assert(string2uint(neg_dec) == -1 * v);
        assert(string2uint(neg_hex) == -1 * v);
    }

    printf(GREENSTR("Pass\n"));
}

int main()
{
    test_string();
    test_float();
}