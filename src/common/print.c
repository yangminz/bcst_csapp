/* BCST - Introduction to Computer Systems
 * Author:      yangminz@outlook.com
 * Github:      https://github.com/yangminz/bcst_csapp
 * Bilibili:    https://space.bilibili.com/4564101
 * Zhihu:       https://www.zhihu.com/people/zhao-yang-min
 * This project (code repository and videos) is exclusively owned by yangminz 
 * and shall not be used for commercial and profitting purpose 
 * without yangminz's permission.
 */

#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include "headers/common.h"

// wrapper of stdio printf
// controlled by the debug verbose bit set
uint64_t debug_printf(uint64_t open_set, const char *format, ...)
{
    if ((open_set & DEBUG_VERBOSE_SET) == 0x0)
    {
        return 0x1;
    }

    // implementation of std printf()
    va_list argptr;
    va_start(argptr, format);
    vfprintf(stderr, format, argptr);
    va_end(argptr);

    return 0x0;
}