## Malloc练习题

我们只考虑CSAPP中的`malloc`与`free`，不考虑真实生产环境，例如`glibc`中的`malloc`。

## 函数接口

-   对于`void *malloc(size_t size)`，申请分配的大小`size`，是否需要对齐？

-   对于`void *malloc(size_t size)`，返回的指针是否对齐？

-   对于`void *malloc(size_t size)`返回的Block，其Block Size与参数`size`的关系如何？

-   如果`malloc`无法分配内存，将返回怎样的指针？

## 对比Stack与Heap

-   关于Stack中函数调用的Frame，它的大小是什么时期确定的？编译时期（Compile-Time），还是运行时期（Run-Time）？

-   在Stack上为数据分配内存，我们需要在什么阶段确定数据的大小？Compile-Time还是Run-Time？

-   假定我们需要分配一块内存，但直到Run-Time才能确定所需分配的大小，我们应该在Stack还是Heap上分配它？

-   同样分配512KB的内存，对比Stack和Heap进行分配所需的时间，通常哪一个所需时间更短？

-   对于Stack，进行递归调用时，可能发生怎样的错误？

-   假定函数进行递归调用，并且规避了无限递归，但在递归调用时通过`malloc`分配内存，可能发生怎样的错误？

-   无限递归时通过`malloc`分配内存，并且没有释放，假定内存泄漏的速度快于`Stack Overflow`，最终会产生怎样的错误？

-   在死循环`while(1)`中通过`malloc`分配内存，可能发生怎样的错误？

-   如果程序中不存在死循环，对于`malloc`分配的内存，不调用`free`进行释放，最后结束运行。怎样评估这一程序在内存泄漏（Memory Leak）方面的风险？

## Heap的基本结构

-   Heap的Prologue与Epilogue的Block Size分别是？

-   Prologue与Epilogue的Allocation Bit分别是？

-   由于Prologue的地址对齐，Heap中有多少Byte是未被使用的？

## Block的结构

-   在CSAPP中，Block与多少Byte进行对齐？

-   如果要记录Block Size与Allocation Bit，Block Header的结构如何？

-   由于对齐，Block的起始虚拟地址最低的三个Bit分别是？

-   由于对齐，Block Size的二进制编码中，最低的三个Bit分别是？

-   Payload的起始虚拟地址怎样对齐？

_不考虑8-Byte的Block的情况。假定Block Header中最低位为Allocation Bit。_

-   假如Header的数值为`0x000abcd9`，它的Block Size与Allocation Bit分别为？

-   假如Header的数值为`0x000abcd8`，它的Block Size与Allocation Bit分别为？

-   为什么需要Footer？

-   给定当前Block的指针，如何判断前一个Block（低地址方向）的Block Size以及Allocation Bit？

-   给定当前Block的指针，如何判断后一个Block（高地址方向）的Block Size以及Allocation Bit？

-   同时存在Header与Footer的情况下，同时考虑Padding，一个Allocated Block，它的内存利用率最大与最小为多少？计算结果应当是与Block Size有关的函数。

-   Implicit Free List中，Block Size最小是多少？

-   Explicit Free List中，Block Size最小是多少？

-   Red-Black Tree中，Block Size最小是多少？

_考虑8-Byte的Block，且假定Header为：`High address ... P8, B8, Allocated/Free`。_

-   最低三位`P8, B8, AF`的八种情况：`000, 001, 010, 011, 100, 101, 110, 111`，分别描述了什么？

-   考虑相邻的Allocated Block，它们的Block Size为：`Low address ... 32, 8, 16, 8, 8, 16, 1024, 8, 8`，它们的header是怎样的？

-   给定当前Block的指针，如何判断前一个Block（低地址方向）的Block Size？注意，当前Block与前一个Block的Block Size都可能是8 Bytes。

-   给定当前Block的指针，如何判断后一个Block（高地址方向）的Block Size？注意，当前Block与前一个Block的Block Size都可能是8 Bytes。

-   8-Byte Allocated Block的内存利用率最大和最小为多少？计算结果应当是常数。

## Free Block搜索

-   对比Explicit Free List与Implicit Free List，假定它们采用同一种适配策略（例如同样Best Fit或同样First Fit），通常哪一个更快搜索到合适的Free Block？

-   如果均采用Best Fit，且Explicit Free List搜索时间为Implicit Free List搜索时间的两倍，Heap应有怎样的布局？

-   如果均采用Best Fit，且Explicit Free List搜索时间与Implicit Free List搜索时间相等，Heap应有怎样的布局？

-   如果均采用Best Fit，在什么情况下，Segregated Free List与Explicit Free List搜索时间相等？

-   对比Best Fit + Explicit Free List，Red-Black Tree的搜索时间如何？

## Free Block分割

-   假如Free Block发生分割，对比Explicit Free List与Red-Black Tree，哪一个需要更长的时间？

_考虑8-Byte对齐，M<=N，均为正整数。适配到8M Bytes的Free Block，请求分配8N Bytes的Block。_

_以下情况**不考虑**8-Byte Block。_

-   Implicit Free List，M=4, N=3, 分割后所得的Free Block是否可能在将来被标记为Allocated？

-   Explicit Free List，M=4, N=3, 是否发生分割？

-   Explicit Free List，M=4, N=2, 分割后所得的Free Block是否可能在将来被标记为Allocated？

-   Red-Black Tree，M=6, N=4, 是否发生分割？

-   Red-Black Tree，M=6, N=3, 分割后所得的Free Block是否可能在将来被标记为Allocated？

_以下情况**考虑**8-Byte Block。_

-   M与N有怎样的关系时，发生分割？

-   假定发生分割，分割后所得的Free Block是否可能在将来被标记为Allocated？

## Heap扩张

-   `malloc`时，进程发现Heap现有的虚拟空间（Virtual Memory Area）不够，通过哪一个系统调用，向操作系统申请内存？

-   `sbrk`或`brk`做了哪些工作？

-   假如Last Block为Allocated，扩张后，被分配的Block的起始虚拟地址是？

-   假如Last Block为Free，扩张后，被分配的Block的起始虚拟地址是？

## Free

-   仅考虑Implict Free List，`free`的时间复杂度是多少？

-   仅考虑Explicit Free List，`free`的时间复杂度是多少？

-   仅考虑Segregated Free List，`free`的时间复杂度是多少？

-   仅考虑Red-Black Tree，`free`的时间复杂度是多少？

-   为什么`free`需要合并前后的Free Block？

-   哪些情况需要在`free`时需要合并（Coalescing）前后Block？

-   每一次完成`malloc`与`free`后，Heap所维护Allocated与Free Block之间的不变关系（Invariant）是？

-   合并以后Block Size是？

-   如果一个Block仅有Header，并且Header仅提供Block Size与Allocation Bit，是否能够与前一个Block（低地址方向）合并？

-   如果一个Block仅有Header，如何设计Header的最低3位，使得它可以搜索到前一个Block（低地址方向）？注意，分为Allocated Block与Free Block两种情况讨论，不考虑8-Byte Block。

## 性能分析

-   对于请求序列R1, R2, ... 如何定义Heap的吞吐率（Throughput）？

-   考虑三种适配策略：Best Fit, First Fit, Next Fit，怎么评价它们对吞吐率的影响？

-   对于请求序列R1, R2, ... 如何定义Heap的内存利用率（Utilization）？

-   对比内存碎片的类型，内部碎片（Internal Fragmentation）与外部碎片（External Fragmentation）分别由什么产生？

-   内部碎片与外部碎片，哪一种更容易估算？

-   考虑两种适配策略：Best Fit, First Fit，怎么评价它们对内存利用率的影响？

-   通过选择适配策略，Explicit Free List如何平衡吞吐率与内存利用率？

-   Segregated Free List如何影响内存利用率？

-   与Best Fit + Explicit Free List相比，Red-Black Tree在吞吐率与内存利用率上有何差异？