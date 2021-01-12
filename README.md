**BCST (Bilibili Computer Science Topics) Project**

*Copyright Declaration*

This repository is a part of BCST project. BCST project contains the source code and videos produced by yangminz. The videos are published publicly on the bilibili by (yaaangmin): [https://space.bilibili.com/4564101]. The source code are published on github (yangminz): [https://github.com/yangminz]. Any repository and video under BCST project is exclusively owned by yangminz and shall not be used for commercial and profitting purpose without yangminz's permission.

**Foundation of Computer System - Computer System A Programmer's Perspective (CSAPP)**

This repository aims to give an introduction to the computer science text book: *Computer System A Programmer's Perspective (CSAPP)*. In this repository, we will talk about the basic components of computer systems. The following is the table of contents:

**Representing and Manipulating Information**

`0x00` Introduction to system resources - The representation of signed and unsigned integers, and the algebra structure behind them: [https://www.bilibili.com/video/BV1mp4y1a7X4/] (2020-09-29)

`0x01` Write basic C language program - Find the low `1` bit and check if one hex integer is all composed of letters; The segment array data structure: [https://www.bilibili.com/video/BV1Hi4y1E7Bn/] (2020-10-01)

`0x02` Floating point numbers - The 4 forms of floating point numbers and the rounding rule. Type converting from unsigned value to its floating representation: [https://www.bilibili.com/video/BV1vy4y1C75t/] (2020-10-16)

**Machine-Level Representation of Program**

`0x03` Assembly simulator - The resources inside CPU: registers. The first step to simulating a program, i.e. describte the registers inside CPU: [https://www.bilibili.com/video/BV1vy4y1C75t/] (2020-10-22)

`0x04` Decode the instructions - The data structure to hold the information of one instruction, i.e. the operand structure and how can we interpret it: [https://www.bilibili.com/video/BV1Na4y1s7m6/] (2020-10-31)

`0x05` One CPU instruction cycle - The execution of one instruction: how does one Von Neumann computer uses CPU registers and memory: [https://www.bilibili.com/video/BV1sZ4y1V7uu/] (2020-11-05)

`0x06` First run of the simulator - The interface to access memory and `call` instruction: [https://www.bilibili.com/video/BV1vK4y1E7sy/] (2020-11-06)

`0x07` The relationship between Recursion and Tree and Stack - `push` instruction: [https://www.bilibili.com/video/BV1tz4y1y7JZ/] (2020-11-18)

`0x08` First refactory of the code base - complete `pop` and `ret` instructions, converting from im-memory instruction structures to string encoded assembly instructions: [https://www.bilibili.com/video/BV1WK41137JT/] (2020-12-04)

`0x09` Use Deterministic Finite Automata to parse the immediate number string, applicable for future ELF file parsing: [https://www.bilibili.com/video/BV1ty4y1S7z1/] (2020-12-08)

`0x0A` Parse the stringfied assembly operands: Register and Memory formats: [https://www.bilibili.com/video/BV1zT4y1M7GE] (2020-12-18)

`0x0B` End of parsing instructions; Conditional codes in CPU [https://www.bilibili.com/video/BV1Qf4y1e73j] (2020-12-27)

`0x0C` End of Part 1: jmp, jne, leave instructions; Completion of recursive function call [https://www.bilibili.com/video/BV1Ey4y1v7aG] (2021-01-04)

**References**

Refer to these books for deeper understanding, as well as better implementation:

1. **Computer Systems: A Programmer's Perspective, Third Edition**, *Randal E. Bryant and David R. O'Hallaron*, Carnegie Mellon University

This is the main text book we are using overall.

2. **Operating System Concepts, Tenth Edition**, *Avi Silberschatz, Peter Baer Galvin and Greg Gagne*, John Wiley & Sons, Inc.

This is a famous OS textbook. We will use this textbook for better understanding on process and virtual memory.

3. **Operating Systems: Three Easy Pieces**, *Remzi H. Arpaci-Dusseau and Andrea C. Arpaci-Dusseau* (University of Wisconsin-Madison)

The authors are couple. They publish this textbook [free online](http://pages.cs.wisc.edu/~remzi/OSTEP/). We are implementing several components from this textbook: process context switching, virutal memory swapping.

3. **Linker and Libraries Guide**, *Sun Microsystems, Inc.*

This guide can help us specify the format of Executable and Linkable Formats and implement the static linking: symbol processing, simple resolution, relocation processing.

5. **Compilers: Principles, Techniques, and Tools**, *Alfred V. Aho, Monica S. Lam, Ravi Sethi, and Jeffrey D. Ullman*, Pearson Education, Inc

We use this compiler's textbook to implement DFA for type converter: `uint64_t string2uint(const char *str)`.

6. **Understanding the Linux Kernel, Second Edition**, *Daniel P. Bovet and Marco Cesati*

This book describes the Linux kernel in detail, will help our understanding of Linux.

7. **Windows Internals Seventh Edition**, *Pavel Yosifovich, Alex Ionescu, Mark E. Russinovich, David A. Solomon*, Microsoft Press

It's good to see the implementation of another OS.