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
#include <stdint.h>
#include <string.h>
#include "headers/common.h"

// covert string to int64_t
uint64_t string2uint(const char *str)
{
    return string2uint_range(str, 0, -1);
}

inline uint64_t increase_digit(int64_t x, char c)
{
    // x - the current int64_t bitmap
    // c - the char to be added at low digit
    int64_t abs = x < 0 ? -1 * x : x;
    abs = abs * 10 + c - '0';
    if (x < 0)
    {
        return -1 * abs;
    }
    else
    {
        return abs;
    }
}

string2uint_state_t string2uint_next(string2uint_state_t state, char c, uint64_t *bmap)
{
    // state - parsing state. see the common.h marcos
    // bmap - the bitmap of the value
    // return value - next state
    switch (state)
    {
        case LEADING_SPACE:
            if (c == '0')
            {
                // 1. positive dec value with leading zeros
                // 2. hex number (positive only)
                *bmap = 0;
                return FIRST_ZERO;
            }
            else if ('1' <= c && c <= '9')
            {
                // positive dec
                *bmap = c - '0';
                return POSITIVE_DEC;
            }
            else if (c == '-')
            {
                // signed negative value
                return NEGATIVE_DEC;
            }
            else if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            {
                // skip leading spaces
                return LEADING_SPACE;
            }
            return FAILED_TRANSFER;
        case FIRST_ZERO:
            // check zero
            if ('0' <= c && c <= '9')
            {
                // no overflow here
                *bmap = (*bmap) * 10 + c - '0';
                return POSITIVE_DEC;
            }
            else if (c == 'x' || c == 'X')
            {
                // we do not have negative value for hex
                return POSITIVE_HEX;
            }
            else if (c == ' ')
            {
                // zero only
                assert(*bmap == 0);
                return ENDING_SPACE;
            }
            return FAILED_TRANSFER;
        case POSITIVE_DEC:
            // dec number
            // signed or unsigned
            if ('0' <= c && c <= '9')
            {
                uint64_t x = *bmap;
                x = x * 10 + c - '0';
                // check unsigned overflow
                if (x < *bmap)
                {
                    // unsigned overflow
                    return SIGNED_OVERFLOW;
                }
                *bmap = x;
                return POSITIVE_DEC;
            }
            else if (c == ' ')
            {
                return ENDING_SPACE;
            }
            // fail
            return FAILED_TRANSFER;
        case NEGATIVE_DEC:
            // negative
            // set the bit map of the negative value till now
            if ('0' <= c && c <= '9')
            {
                // negative value should not have leading zeros
                // safe for negative multiplication
                uint64_t bmap_2x = (*bmap) << 1;
                uint64_t bmap_8x = (*bmap) << 3;
                // this works for leading zeros: -0000...
                uint64_t x = bmap_2x + bmap_8x + 1 + ~(c - '0');
                if (((x >> 63) == 0) && ((*bmap >> 63) == 1))
                {
                    return SIGNED_OVERFLOW;
                }
                *bmap = x;
                return NEGATIVE_DEC;
            }
            // fail
            return FAILED_TRANSFER;
        case POSITIVE_HEX:
            // hex number
            if ('0' <= c && c <= '9')
            {
                *bmap = (*bmap) * 16 + c - '0';
                return POSITIVE_HEX;
            }
            else if ('a' <= c && c <= 'f')
            {
                *bmap = (*bmap) * 16 + c - 'a' + 10;
                return POSITIVE_HEX;
            }
            else if ('A' <= c && c <= 'F')
            {
                *bmap = (*bmap) * 16 + c - 'A' + 10;
                return POSITIVE_HEX;
            }
            else if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            {
                return ENDING_SPACE;
            }
            // fail
            return FAILED_TRANSFER;
        case ENDING_SPACE:
            if (c == ' ' || c == '\t' || c == '\r' || c == '\n')
            {
                // skip tailing spaces
                return ENDING_SPACE;
            }
            // fail
            return FAILED_TRANSFER;
        default:
            // fail
            return FAILED_TRANSFER;
    }
}

uint64_t string2uint_range(const char *str, int start, int end)
{
    // start: starting index inclusive
    // end: ending index includsive
    end = (end == -1) ? strlen(str) - 1 : end;

    uint64_t uv = 0;

    // DFA: deterministic finite automata to scan string and get value
    string2uint_state_t state = LEADING_SPACE;

    for (int i = start; i <= end; ++ i)
    {
        char c = str[i];
        state = string2uint_next(state,c, &uv);

        switch (state)
        {
            case FAILED_TRANSFER:
#ifdef DEBUG_STRING2UINT
                printf("string2uint: failed to transfer: %s\n", str);
#endif
                exit(0);
            case UNSIGNED_OVERFLOW:
#ifdef DEBUG_STRING2UINT
                printf("string2uint: unsigned overflow: %s\n", str);
#endif
                exit(0);
            case SIGNED_OVERFLOW:
#ifdef DEBUG_STRING2UINT
                printf("string2uint: signed overflow: %s\n", str);
#endif
                exit(0);
            default:
                break;
        }
    }

    return uv;
}

// convert uint32_t to its float
uint32_t uint2float(uint32_t u)
{
    if (u == 0x00000000)
    {
        return 0x00000000;
    }
    // must be NORMALIZED
    // counting the position of highest 1: u[n]
    int n = 31;
    while (0 <= n && (((u >> n) & 0x1) == 0x0))
    {
        n = n - 1;
    }
    uint32_t e, f;
    //    seee eeee efff ffff ffff ffff ffff ffff
    // <= 0000 0000 1111 1111 1111 1111 1111 1111
    if (u <= 0x00ffffff)
    {
        // no need rounding
        uint32_t mask = 0xffffffff >> (32 - n);
        f = (u & mask) << (23 - n);
        e = n + 127;
        return (e << 23) | f;
    }
    // >= 0000 0001 0000 0000 0000 0000 0000 0000
    else
    {
        // need rounding
        // expand to 64 bit for situations like 0xffffffff
        uint64_t a = 0;
        a += u;
        // compute g, r, s
        uint32_t g = (a >> (n - 23)) & 0x1;     // Guard bit, the lowest bit of the result
        uint32_t r = (a >> (n - 24)) & 0x1;     // Round bit, the highest bit to be removed
        uint32_t s = 0x0;                       // Sticky bit, the OR of remaining bits in the removed part (low)
        for (int j = 0; j < n - 24; ++ j)
        {
            s = s | ((u >> j) & 0x1);
        }
        // compute carry
        a = a >> (n - 23);
        // 0    1    ?    ... ?
        // [24] [23] [22] ... [0]
        /* Rounding Rules
            +-------+-------+-------+-------+
            |   G   |   R   |   S   |       |
            +-------+-------+-------+-------+
            |   0   |   0   |   0   |   +0  | round down
            |   0   |   0   |   1   |   +0  | round down
            |   0   |   1   |   0   |   +0  | round down
            |   0   |   1   |   1   |   +1  | round up
            |   1   |   0   |   0   |   +0  | round down
            |   1   |   0   |   1   |   +0  | round down
            |   1   |   1   |   0   |   +1  | round up
            |   1   |   1   |   1   |   +1  | round up
            +-------+-------+-------+-------+
        carry = R & (G | S) by K-Map
        */
        if ((r & (g | s)) == 0x1)
        {
            a = a + 1;
        }
        // check carry
        if ((a >> 23) == 0x1)
        {
            // 0    1    ?    ... ?
            // [24] [23] [22] ... [0]
            f = a & 0x007fffff;
            e = n + 127;
            return (e << 23) | f;
        }
        else if ((a >> 23) == 0x2)
        {
            // 1    0    0    ... 0
            // [24] [23] [22] ... [0]
            e = n + 1 + 127;
            return (e << 23);
        }
    }
    // inf as default error
    return 0x7f800000;
}