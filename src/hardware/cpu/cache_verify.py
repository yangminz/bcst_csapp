#!/usr/bin/python3
# usage: 
#   ./cmd.py argv[1] argv[2], ...
#   /usr/bin/python3 ./cmd.py argv[1] argv[2], ...
import sys
import os
import subprocess

print("================\ntest cache")
i = 0
for argv in sys.argv:
    print("[", i, "] ", argv)
    i += 1
print("================")

assert(len(sys.argv) == 3)

csim_ref_file = sys.argv[1]
trace_path = sys.argv[2]

assert(os.path.isfile(csim_ref_file))
assert(os.path.isdir(trace_path))

test_cases = [
    # s E b
    [   2,  1,      2,  "wide.trace"    ],
    [   3,  2,      2,  "load.trace"    ],
    [   1,  1,      1,  "yi2.trace" ],
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
    a = [
        "/usr/bin/python3",
        "./src/hardware/cpu/test_cache.py",
        "/mnt/e/Ubuntu/cache/csim-ref",
        "/mnt/e/Ubuntu/cache/traces/" + file,
        str(s), str(E), str(b),
    ]
    print(" ".join(a))

    subprocess.run(a)