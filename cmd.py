#!/usr/bin/python3
# usage: 
#   ./cmd.py argv[1] argv[2], ...
#   /usr/bin/python3 ./cmd.py argv[1] argv[2], ...
import sys, os, subprocess, argparse, math, json
from pathlib import Path
from functools import reduce

parser = argparse.ArgumentParser()
parser.add_argument("-b", "--build", help="Build the target module.")
parser.add_argument("-t", "--test", help="Test the target module.")
parser.add_argument("-d", "--debug", help="Debug the target module.")
parser.add_argument("-m", "--memcheck", help="Check the memory of the target module.")
parser.add_argument("--hex", help="Convert ascii word to hex 64 numbers.")
parser.add_argument("--clean", action="store_true", help="Clean the unused files.")
parser.add_argument("--count", action="store_true", help="Count the lines of source code.")
parser.add_argument("--format", action="store_true", help="Format the lines of source code.")
parser.add_argument("--copyright", action="store_true", help="Add copyright info to source code.")
parser.add_argument("--csim", action="store_true", help="Check and verify the cache module.")
args = parser.parse_args()

# print arguments
i = 0
for argv in sys.argv:
    print("[", i, "] ", argv)
    i += 1
print("================")

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
    if not os.path.isdir("./files/swap/"):
        os.mkdir("./files/swap/")

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
        # skip the test files
        if "tests" in str(filename):
            continue
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

def load_config():
    filename = "./config.json"
    content = open(filename, "r", encoding="utf-8").read()
    js = json.loads(content)
    return js

def run_recursive(obj):
    if isinstance(obj, list):
        if len(obj) > 0:
            all_childs_string = reduce(
                lambda x, y: x and y, 
                list(map(lambda x: isinstance(x, str), obj)))
            list_childs = list(filter(lambda x: isinstance(x, list), obj))
            if all_childs_string:
                # a list of strings -- command
                print(" ".join(obj))
                subprocess.run(obj)
            elif len(list_childs) > 0:
                for child in list_childs:
                    run_recursive(child)
    
def execute(module, component):
    make_build_directory()
    config = load_config();
    if not module in config:
        print("input the correct build module:", config.keys())
        exit()
    if not component in config[module]:
        print("target module '" + module + "' does not have the '" + component + "' component.\nplease check.")
        exit()
    # find all list of strings and execute
    assert(component in {"build", "test", "debug", "memcheck"})
    run_recursive(config[module][component])

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
            "./src/tests/test_cache.py",
            "/mnt/e/Ubuntu/cache/csim-ref",
            "/mnt/e/Ubuntu/cache/traces/" + file,
            str(s), str(E), str(b),
        ]
        print(" ".join(a))
        subprocess.run(a)

def printHex():
    strVal = args.hex
    print(strVal)
    length = len(strVal)
    buffer = ""
    for i in range(math.ceil(length/8)):
        line = ""
        for j in range(8):
            index = j + i * 8
            if index < len(strVal):
                line = str(hex(ord(strVal[index])))[2:] + line
            else:
                line = "00" + line
        line = "[" + str(i * 8) + "]\t" + line
        line += "\t" + strVal[i * 8: i * 8 + 8] + "\n"
        buffer += line
    print(buffer)

# main

if args.build:
    execute(args.build, "build")
elif args.test:
    execute(args.test, "test")
elif args.debug:
    execute(args.debug, "debug")
elif args.memcheck:
    execute(args.memcheck, "memcheck")
elif args.hex:
    printHex()
elif args.count == True:
    count_lines()
elif args.copyright == True:
    add_copyright_header()
elif args.format == True:
    format_code()
elif args.csim == True:
    cache_verify()