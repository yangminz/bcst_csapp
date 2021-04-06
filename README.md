**BCST (Bilibili Computer Science Topics) Project**

*Copyright Declaration*

This repository is a part of BCST project. BCST project contains the source code and videos produced by yangminz. The videos are published publicly on the bilibili by (yaaangmin): [https://space.bilibili.com/4564101]. The source code are published on github (yangminz): [https://github.com/yangminz]. Any repository and video under BCST project is exclusively owned by yangminz and shall not be used for commercial and profitting purpose without yangminz's permission.

**计算机系统 - Computer System A Programmer's Perspective (CSAPP)**

Hello 观众朋友们大家好，这个Repo是我在做《深入理解计算机系统》（CSAPP）视频时所用到的代码，还包括一本设计手册，设计手册还在慢慢更新。代码、手册以及视频都是为了给大家介绍CSAPP这本书，帮助大家阅读和理解最基础的计算机系统知识。特别是本科低年级的同学（大一大二），准备转专业的同学，以及跨专业考研的同学，通常会掌握一些基本的编程语言，但是可能对计算机系统本身并不足够了解。我的视频、代码以及手册就是希望帮助大家学习，尽量通过代码实现CSAPP上的知识点，然后再通过视频讲解，将基础的计算机系统知识连贯起来。

下面是目录：

#### Chapter 02 - Representing and Manipulating Information

计算机可以看作对Bit串的翻译器，为了了解计算机系统，我们首先需要了解Bit在计算机中是如何被表达与解释的，简单了解这些位运算的操作，背后的代数结构。

**2020-09-29** 关于有符号与无符号整数，以及它们背后的代数结构。我设计了一张环形图，方便观众理解Bit串到有符号与无符号整数的映射。这张环形图在手册里更加精美： [https://www.bilibili.com/video/BV1mp4y1a7X4/]

**2020-10-01** 我们第一次动手写C语言代码。一些简单的位操作运算，例如判断一个十六进制数中是否全部都是字母，使用卡诺图对布尔代数进行化简。一个基于位运算的数据结构：树状数组。[https://www.bilibili.com/video/BV1Hi4y1E7Bn/]

**2020-10-16** 浮点数的四种类型：规格化、非规格化、无穷大、非数。`uint32_t`到`float`的数值转换，特别是关于约分的规定：[https://www.bilibili.com/video/BV1vy4y1C75t/]

#### Chapter 03 - Machine-Level Representation of Program

拥有Bit构成的基本数据类型以后，我们开始考虑构造一台原始的冯诺依曼结构的计算机。其实，我们是在做一个计算机的模拟器。为此，我们需要模拟CPU中的寄存器与内存等硬件，在这些硬件的基础上编写指令集。我们的指令集是通过字符串写成的，就像我们通常所写的高级语言代码一样，其实都是文本。我们用一些编译的基础知识来解析字符串指令集，将它们翻译成指令的数据类型，然后按照操作符与操作数解释执行这些代码。

在这里，我们其实做了一层抽象：文本与字符串指令就像一台虚拟机，类似于JVM中的字节码，CLR中的IL，我们解释执行的是虚拟机指令。字符串指令集上，我们实现一些简单的指令，例如`add`，`sub`，`mov`，最重要的是关于过程控制，也就是函数调用的几个指令：`call`，`push`，`pop`，`ret`，我们对内存栈的控制蕴含在这几条指令之中。

**2020-10-22** CPU寄存器的模拟器，这是我们模拟器的第一行代码。我们利用结构体与联合等异构的数据类型来描述寄存器：[https://www.bilibili.com/video/BV1vy4y1C75t/]

**2020-10-31** 指令的数据结构，用来描述一条汇编指令的操作符，源操作数以及目的操作数，包括操作数中的立即数、寄存器等：[https://www.bilibili.com/video/BV1Na4y1s7m6/]

**2020-11-05** 一个CPU的指令周期，我们第一次执行了一条汇编指令：[https://www.bilibili.com/video/BV1sZ4y1V7uu/]

**2020-11-06** 模拟器的运行若干条指令，形成一个程序。为了访问内存资源，我们需要设计内存访问接口。完成`call`指令：[https://www.bilibili.com/video/BV1vK4y1E7sy/]

**2020-11-18** 递归与树与栈之间的关系，实现`push`指令：[https://www.bilibili.com/video/BV1tz4y1y7JZ/]

**2020-12-04** 完成`pop`与`ret`指令，至此，我们已经在内存栈上实现了函数的过程控制。我们开始第一次代码重构，将in-memory的结构体指令数组转换为字符串指令数组，并且开始解析字符串指令：[https://www.bilibili.com/video/BV1WK41137JT/]

**2020-12-08** 为了解析字符串指令，特别是其中所要用到的立即数，我们开始写字符串到十进制与十六进制数的转换。我们使用确定有限自动状态机去扫描字符串，设计状态机的状态转移：[https://www.bilibili.com/video/BV1ty4y1S7z1/]

**2020-12-18** 解析字符串操作数中的内存与寄存器格式：[https://www.bilibili.com/video/BV1zT4y1M7GE]

**2020-12-27** 完成字符串指令的解析，并且在字符串指令上运行了程序。设计状态码，支持条件判断：[https://www.bilibili.com/video/BV1Qf4y1e73j]

**2021-01-04** 实现`jmp`，`jne`，`leave`指令。到此为止，我们已经可以模拟函数的递归调用了：[https://www.bilibili.com/video/BV1Ey4y1v7aG]

**2021-01-15** 总结与回顾我们到此为止所做的一切工作，关于数据类型和冯诺依曼计算机，一个简化的计算模型——URM：[https://www.bilibili.com/video/BV1LU4y14792/]

**2021-01-23** 除了冯诺依曼计算机，我们还有另一种计算模型：λ演算。冯诺依曼计算机中，函数是数据；λ演算中，数据是函数：[https://www.bilibili.com/video/BV1Wv411s7yU/]

#### Chapter 07 - Linking

模拟了计算机的硬件基础以后，我们准备开始在模拟器上运行程序。因此，我们需要设计自己的程序。考虑到我们将指令集用字符串实现，因此我们的程序也是字符串的汇编指令。这一步工作可以由编译器为我们实现，我们需要做的是完成一个简陋的链接器，将可执行的文件链接起来。

正如我们在上一部分所说的，字符串指令可以看作一种在字符串与文本上实现的虚拟机，因此我们的链接也是在`.txt`文本上实现的，一个可执行可链接的文本格式，`.elf.txt`。为了设计这个文本格式，我们需要理解Linux中是如何处理ELF文件的。

**2021-02-06** ELF文件从它的Header开始，我们用`readelf`与`hexdump`去看ELF文件的二进制数据，按照Byte去理解ELF文件的Header：[https://www.bilibili.com/video/BV1MX4y1576E/]

**2021-02-21** Section Header Table的结构：[https://www.bilibili.com/video/BV1AU4y1s7Je/]

**2021-02-28** ELF的符号表，以及什么是符号，它们在编译器和链接器视角中的差异：[https://www.bilibili.com/video/BV1VX4y1V7JP/]

**2021-03-07** 符号表中的Bind, Type以及Index，C语言`.c`源文件中不同位置的申明与定义是如何在ELF文件中描述的，特别是COMMON节等容易混淆的地方：[https://www.bilibili.com/video/BV14h411Q7Rz/]

**2021-03-14** 设计可执行与可链接的文本格式，`.elf.txt`。从磁盘上的文件中读取到内存：[https://www.bilibili.com/video/BV1Ji4y1K7uw/]

**2021-03-21** 解析`.elf.txt`的节头表与符号表：[https://www.bilibili.com/video/BV1Nz4y1279C/]

**2021-03-27** 静态链接的第一步：符号解析，设计内部的符号表作为维护符号关系的数据结构：[https://www.bilibili.com/video/BV1EZ4y1A7qM/]

**2021-04-04** 静态链接的第二步：将ELF的Section合并为EOF的Segment，并且计算Segment的运行时起始地址：[https://www.bilibili.com/video/BV1Lh411D7fh/]
