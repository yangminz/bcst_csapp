import os
from pathlib import Path

# file headers contains copyright information
notification = "/* BCST - Introduction to Computer Systems\n" \
" * Author:      yangminz@outlook.com\n" \
" * Github:      https://github.com/yangminz/bcst_csapp\n" \
" * Bilibili:    https://space.bilibili.com/4564101\n" \
" * Zhihu:       https://www.zhihu.com/people/zhao-yang-min\n" \
" * This project (code repository and videos) is exclusively owned by yangminz \n" \
" * and shall not be used for commercial and profitting purpose \n" \
" * without yangminz's permission.\n" \
" */\n\n"

# get files with paths
filelist = list(Path(".").rglob("*.[ch]"))

# recursively add lines to every .c and .h file
print("recursively add lines to every .c and .h file")
for filename in filelist:
    with open(filename, "r", encoding = 'utf-8') as fr:
        content = fr.read()
        if (content.startswith("/* BCST - Introduction to Computer Systems")):
            print("\tskip\t%s" % filename)
            fr.close()
            continue
        else:
            fr.close()
            # reopen and write data: this is a safer approach
            # try to not open in r+ mode
            print("\tprepend\t%s" % filename)
            with open(filename, "w", encoding = 'utf-8') as fw:
                fw.write(notification + content)