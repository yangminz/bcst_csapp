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

总结一：数据类型与计算模型
[https://www.bilibili.com/video/BV1LU4y14792/](2021-01-15)

总结二：递归与λ表达式
[https://www.bilibili.com/video/BV1Wv411s7yU/](2021-01-23)

## 第七章链接 - 一个简陋的静态文本链接器实现

ELF文件的格式 - Header
[https://www.bilibili.com/video/BV1MX4y1576E/](2021-02-06)

Section Header Table
[https://www.bilibili.com/video/BV1AU4y1s7Je/](2021-02-21)

ELF符号表
[https://www.bilibili.com/video/BV1VX4y1V7JP/](2021-02-28)

ELF符号表 - Bind, Type, Index
[https://www.bilibili.com/video/BV14h411Q7Rz/](2021-03-07)

可执行可连接的`.elf.txt`文本格式
[https://www.bilibili.com/video/BV1Ji4y1K7uw/](2021-03-14)

解析`.elf.txt`的节头表与符号表
[https://www.bilibili.com/video/BV1Nz4y1279C/](2021-03-21)

符号解析
[https://www.bilibili.com/video/BV1EZ4y1A7qM/](2021-03-27)

Section合并，计算Run-time的Segment地址
[https://www.bilibili.com/video/BV1Lh411D7fh/](2021-04-04)