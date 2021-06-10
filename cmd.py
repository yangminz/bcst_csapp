#!/usr/bin/python3
# usage: 
#   ./cmd.py argv[1] argv[2], ...
#   /usr/bin/python3 ./cmd.py argv[1] argv[2], ...
import sys
import os
import subprocess
from pathlib import Path

# print arguments
i = 0
for argv in sys.argv:
    print("[", i, "] ", argv)
    i += 1
print("================")

KEY_MACHINE = "m"

EXE_BIN_MACHINE = "./bin/test_machine"

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

def make_build_directory():
    if not os.path.isdir("./bin/"):
        os.mkdir("./bin/")

def add_copyright_header():
    # get files with paths
    filelist = list(Path(".").rglob("*.[ch]"))

    # recursively add lines to every .c and .h file
    print("recursively add lines to every .c and .h file")
    for filename in filelist:
        try:
            with open(filename, "r", encoding = 'ascii') as fr:
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
                    with open(filename, "w", encoding = 'ascii') as fw:
                        fw.write(notification + content)
                        fw.close()
        except UnicodeDecodeError:
            print(filename)

def format_include(s, line_index):
    a = "#include<headers/"
    b = "#include<"
    update = False
    old = s

    # check include
    if s.startswith(a):
        s = "#include \"headers/" + s[len(a):]
        for j in range(len(s)):
            if s[j] == '>':
                l = list(s)
                l[j] = "\""
                s = "".join(l)
        update = True
    elif s.startswith(b):
        s = "#include <" + s[len(b):]
        update = True
    
    if update:
        print("\tline [%d] #include rule: \"%s\" ==> \"%s\"" % (line_index, old, s))
    return s

def format_marco(s, line_index):
    a = s.strip("\n")
    a = a.strip(" ")
    if (len(a) >= 1 and a[len(a) - 1] == ";"):
        return s
    if (a.startswith("#if") or a.startswith("#endif") or a.startswith("#else") or a.startswith("#define")):
        a = a + "\n"
        print("\tline [%d] marco rule: \"%s\" ==> \"%s\"" % (line_index, s, a))
        return a
    return s

def format_tag(s, line_index):
    a = s.strip("\n")
    a = a.strip(" ")
    if (len(a) >= 1 and a[len(a) - 1] == ";"):
        return s
    charset = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "_" }
    for i in range(len(a) - 1):
        if not a[i] in charset:
            return s
    # are all in char set
    if len(a) >= 1 and a[len(a) - 1] == ":":
        # this is a tag
        a = a + "\n"
        print("\tline [%d] tag rule: \"%s\" ==> \"%s\"" % (line_index, s, a))
        return a
    return s

def format_whiteline(s, line_index):
    space = 0
    for c in s:
        if c == ' ':
            space += 1
    if space == len(s) - 1 and s[-1] == '\n':
        s = "\n"
        # print("\tline [%d] white line rule: delete" % (line_index))
    return s

def format_code():
    # get files with paths
    filelist = list(Path(".").rglob("*.[ch]"))
    # recursively add lines to every .c and .h file
    print("recursively check every .c and file")
    for filename in filelist:
        try:
            with open(filename, "r", encoding = 'ascii') as fr:
                content = fr.readlines()
                print(filename, ":")
                for i in range(len(content)):
                    content[i] = format_include(content[i], i)
                    content[i] = format_whiteline(content[i], i)
                    content[i] = format_marco(content[i], i)
                    content[i] = format_tag(content[i], i)
                fr.close()
                # reopen and write data: this is a safer approach
                # try to not open in r+ mode
                with open(filename, "w", encoding = 'ascii') as fw:
                    fw.writelines(content)
                    fw.close()
        except UnicodeDecodeError:
            print(filename)

def count_lines():
    # get files with paths
    filelist = list(Path(".").rglob("*.[ch]"))
    name_count = []
    total_count = 0
    maxfilename = 0
    for filename in filelist:
        count = 0
        for index, line in enumerate(open(filename, 'r')):
            count += 1
        name_count += [[str(filename), count]]
        total_count += count
        if len(str(filename)) > maxfilename:
            maxfilename = len(str(filename))
    # print result
    print("count .c and .h file lines:")
    sortedlist = sorted(name_count, key = lambda x: x[1], reverse=True)
    for [filename, count] in sortedlist:
        print(filename, end="")
        n = (int(maxfilename / 4) + 1) * 4
        for i in range(n - len(filename)):
            print(" ", end="")
        print(count)
    print("\nTotal:", total_count)

def build(key):
    make_build_directory()
    gcc_map = {
        KEY_MACHINE : [
                [
                    "/usr/bin/gcc-7", 
                    "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-function",
                    "-I", "./src",
                    "./src/mains/test_machine.c",
                    "./src/common/print.c",
                    "./src/common/convert.c",
                    "./src/common/cleanup.c",
                    "./src/algorithm/hashtable.c",
                    "./src/algorithm/trie.c",
                    "./src/algorithm/array.c",
                    "./src/hardware/cpu/isa.c",
                    "./src/hardware/cpu/mmu.c",
                    "./src/hardware/memory/dram.c",
                    "-o", EXE_BIN_MACHINE
                ]
            ],
        "link" : [
                [
                    "/usr/bin/gcc-7", 
                    "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-function",
                    "-I", "./src",
                    "-shared", "-fPIC",
                    "./src/common/print.c",
                    "./src/common/convert.c",
                    "./src/common/tagmalloc.c",
                    "./src/common/cleanup.c",
                    "./src/algorithm/array.c",
                    "./src/algorithm/hashtable.c",
                    "./src/algorithm/linkedlist.c",
                    "./src/linker/parseElf.c",
                    "./src/linker/staticlink.c",
                    "-o", "./bin/staticLinker.so"
                ],
                [
                    "/usr/bin/gcc-7", 
                    "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-function",
                    "-I", "./src",
                    "./src/common/print.c",
                    "./src/common/convert.c",
                    "./src/common/tagmalloc.c",
                    "./src/common/cleanup.c",
                    "./src/algorithm/array.c",
                    "./src/algorithm/hashtable.c",
                    "./src/algorithm/linkedlist.c",
                    "./src/linker/linker.c", 
                    "-ldl", "-o", "./bin/link"
                ],
            ],
        "mesi" : [
                [
                    "/usr/bin/gcc-7", 
                    "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-but-set-variable",
                    "-I", "./src",
                    # "-DDEBUG",
                    "./src/mains/mesi.c",
                    "-o", "./bin/mesi"
                ],
            ],
        "false_sharing" : [
                [
                    "/usr/bin/gcc-7", 
                    "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-but-set-variable", "-Wno-unused-variable",
                    "-I", "./src",
                    "-pthread",
                    "./src/mains/false_sharing.c",
                    "-o", "./bin/false_sharing"
                ],
            ],
        "rb" : [
                [
                    "/usr/bin/gcc-7", 
                    "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-but-set-variable", "-Wno-unused-variable", "-Wno-unused-function",
                    "-I", "./src",
                    "-DUNIT_TEST",
                    "./src/algorithm/redblack.c",
                    "-o", "./bin/rb"
                ],
            ],
        "trie" : [
                [
                    "/usr/bin/gcc-7", 
                    "-Wall", "-g", "-O0", "-Werror", "-std=gnu99", "-Wno-unused-but-set-variable", "-Wno-unused-variable", "-Wno-unused-function",
                    "-I", "./src",
                    "-DUNIT_TEST",
                    "./src/algorithm/trie.c", "./src/algorithm/hashtable.c",
                    "-o", "./bin/trie"
                ],
            ],
    }

    if not key in gcc_map:
        print("input the correct build key:", gcc_map.keys())
        exit()
    for command in gcc_map[key]:
        subprocess.run(command)

def run(key):
    assert(os.path.isdir("./bin/"))
    bin_map = {
        KEY_MACHINE : [EXE_BIN_MACHINE],
        "link" : ["./bin/link", "main", "sum", "-o", "output"],
        "mesi" : ["./bin/mesi"],
        "false_sharing" : ["./bin/false_sharing"],
        "rb" : ["./bin/rb"],
        "trie" : ["./bin/trie"],
    }
    if not key in bin_map:
        print("input the correct binary key:", bin_map.keys())
        exit()
    subprocess.run(bin_map[key])

def debug(key):
    assert(os.path.isdir("./bin/"))
    bin_map = {
        KEY_MACHINE : EXE_BIN_MACHINE,
        "link" : "./bin/link"
    }
    if not key in bin_map:
        print("input the correct binary key:", bin_map.keys())
        exit()
    subprocess.run(["/usr/bin/gdb", bin_map[key]])

def mem_check(key):
    assert(os.path.isdir("./bin/"))
    bin_map = {
        KEY_MACHINE : EXE_BIN_MACHINE,
        "link" : "./bin/link"
    }
    if not key in bin_map:
        print("input the correct memory check key:", bin_map.keys())
        exit()
    subprocess.run([
        "/usr/bin/valgrind",
        "--tool=memcheck",
        "--leak-check=full",
        bin_map[key]
    ])

def cache_verify():
    make_build_directory()
    csim_ref_file = "/mnt/e/Ubuntu/cache/csim-ref"
    trace_dir = "/mnt/e/Ubuntu/cache/traces/"

    assert(os.path.isfile(csim_ref_file))
    assert(os.path.isdir(trace_dir))

    test_cases = [
        # s E b
        [   2,  1,      2,  "wide.trace"    ],
        [   3,  2,      2,  "load.trace"    ],
        [   1,  1,      1,  "yi2.trace"     ],
        [   4,  2,      4,  "yi.trace"      ],
        [   2,  1,      4,  "dave.trace"    ],
        [   2,  1,      3,  "trans.trace"   ],
        [   2,  2,      3,  "trans.trace"   ],
        [   14, 1024,   3,  "trans.trace"   ],
        [   5,  1,      5,  "trans.trace"   ],
        [   5,  1,      5,  "long.trace"    ],
    ]

    for [s, E, b, file] in test_cases:
        # need to reload shared library for each test run
        # thus we start a new process
        a = [
            "/usr/bin/python3",
            "./src/mains/cache_verify.py",
            "/mnt/e/Ubuntu/cache/csim-ref",
            "/mnt/e/Ubuntu/cache/traces/" + file,
            str(s), str(E), str(b),
        ]
        print(" ".join(a))
        subprocess.run(a)

# main
assert(len(sys.argv) >= 2)
operation = sys.argv[1].lower()

if operation == "build":
    assert(len(sys.argv) == 3)
    build(sys.argv[2])
elif operation == "run":
    assert(len(sys.argv) == 3)
    run(sys.argv[2])
elif operation == "debug":
    assert(len(sys.argv) == 3)
    debug(sys.argv[2])
elif operation == "memcheck":
    assert(len(sys.argv) == 3)
    mem_check(sys.argv[2])
elif operation == "count":
    count_lines()
elif operation == "clean":
    pass
elif operation == "copyright":
    add_copyright_header()
elif operation == "format":
    format_code()
elif operation == "csim":
    cache_verify()
