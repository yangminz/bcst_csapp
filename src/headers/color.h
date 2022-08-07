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
// such as types, enums and variables
#ifndef COLOR_GUARD
#define COLOR_GUARD

#define REDSTR(STR)     "\033[31;1m"STR"\033[0m"
#define GREENSTR(STR)   "\033[32;1m"STR"\033[0m"
#define YELLOWSTR(STR)  "\033[33;1m"STR"\033[0m"
#define BLUESTR(STR)    "\033[34;1m"STR"\033[0m"

#endif