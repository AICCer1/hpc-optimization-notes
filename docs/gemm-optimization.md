# GEMM Optimization

GEMM（General Matrix Multiply）通常写作：

> **C = A × B + C**

它是高性能计算和 AI 计算里最核心的基础算子之一。

为什么它这么重要？
- BLAS 里最核心的 Level-3 算子之一
- 深度学习里的全连接、卷积降维后常常会映射到 GEMM
- 大量数学库优化能力，最后都会体现在 GEMM 上
- 它非常适合展示 cache、blocking、vectorization、threading、packing 等优化思想

所以很多人学性能优化，最后都会绕回 GEMM。不是它爱出风头，是它确实太基础。

---

## 1. 从最朴素实现开始

最基础的三层循环写法大概是：

```cpp
for (int i = 0; i < M; ++i) {
    for (int j = 0; j < N; ++j) {
        for (int k = 0; k < K; ++k) {
            C[i * N + j] += A[i * K + k] * B[k * N + j];
        }
    }
}
```

这玩意儿的优点是：
- 好懂
- 对
- 能跑

缺点也很明显：
- cache 利用差
- 对 B 的访问经常不连续
- 编译器不一定能很好向量化
- 没有多线程
- 没有数据打包
- 完全没榨干硬件潜力

所以它通常只能作为 baseline。

---

## 2. GEMM 优化到底在优化什么？

核心就两件事：

### 2.1 提高计算单元利用率
也就是尽量让：
- FMA 单元忙起来
- SIMD/vector 单元忙起来
- 多核并行忙起来

### 2.2 提高数据复用，减少低效访存
也就是尽量让：
- 数据多在寄存器里复用
- 再不行也尽量在 L1 / L2 / L3 里复用
- 少去主内存来回拿
- 避免不连续访问和 cache miss

一句人话：

> **别让算力闲着，也别让内存拖后腿。**

---

## 3. 为什么 naive GEMM 慢？

### 3.1 B 的访问模式可能很差
在行主序（row-major）下，如果按：
- `A[i * K + k]`
- `B[k * N + j]`

访问，那么 B 往往是在跨行跳着读。

这会导致：
- 空间局部性差
- cache line 利用率不高
- TLB/cache 压力上升

### 3.2 C 的更新粒度太细
每做一次乘加都在更新一个元素，寄存器复用不充分。

### 3.3 没有 blocking
一个大矩阵直接硬怼，数据很快就把 cache 冲烂了。

### 3.4 没有向量化
现代 CPU 的 SIMD 宽度很大，不用基本等于白交税。

### 3.5 没有并行化
多核 CPU 不并行，约等于拿高配机器当低配用。

---

## 4. 第一层优化：调整循环顺序

三层循环有多种顺序：
- ijk
- ikj
- jik
- jki
- kij
- kji

不同顺序对访存模式影响很大。

比如在 row-major 里，`ikj` 往往比 `ijk` 更容易让：
- A 的访问连续
- B 的某一行访问更连续
- C 的一整行/一段更容易复用

一个更友好的版本大概像这样：

```cpp
for (int i = 0; i < M; ++i) {
    for (int k = 0; k < K; ++k) {
        float a = A[i * K + k];
        for (int j = 0; j < N; ++j) {
            C[i * N + j] += a * B[k * N + j];
        }
    }
}
```

这个版本的好处是：
- `a` 被提到寄存器里复用
- `B[k * N + j]` 连续访问
- `C[i * N + j]` 也是一段连续更新

只是这还远远不够，但已经比最朴素版本顺眼多了。

---

## 5. 第二层优化：Blocking / Tiling

这是 GEMM 优化的核心中的核心。

### 5.1 为什么要 blocking？
因为大矩阵放不进 cache。

如果你整块算：
- A、B、C 的工作集太大
- 算着算着 cache 就被冲掉
- 好不容易读进来的数据来不及复用就没了

所以我们把问题切成小块。

### 5.2 基本思想
把矩阵切成小 tile：
- `A_block`
- `B_block`
- `C_block`

然后以块为单位做乘法累加。

目的是：
- 一个 A block 被复用多次
- 一个 B block 被复用多次
- 一个 C block 尽量常驻 cache / 寄存器

### 5.3 一个简单 blocking 版本

```cpp
for (int ii = 0; ii < M; ii += BM) {
    for (int kk = 0; kk < K; kk += BK) {
        for (int jj = 0; jj < N; jj += BN) {
            for (int i = ii; i < min(ii + BM, M); ++i) {
                for (int k = kk; k < min(kk + BK, K); ++k) {
                    float a = A[i * K + k];
                    for (int j = jj; j < min(jj + BN, N); ++j) {
                        C[i * N + j] += a * B[k * N + j];
                    }
                }
            }
        }
    }
}
```

这里：
- `BM / BK / BN` 就是 tile 大小
- tile 大小不能瞎拍脑袋，要和 cache、寄存器、SIMD 宽度配合

### 5.4 blocking 的本质
它不是“把循环套得更复杂”。

它真正干的是：

> **把原本分散、低复用的访存，变成局部密集复用。**

---

## 6. 第三层优化：Register Blocking / Micro-kernel

高性能 GEMM 一般不满足于 cache blocking，还会继续做：

> **寄存器级 blocking**

意思是：
- 一次算一个小的 `mr x nr` 输出块
- 这个小块的 C 尽量全放在寄存器里累加
- 算完再一次性写回

比如：
- `mr = 8`
- `nr = 8`

这时候一个 micro-kernel 会尽量：
- 把 A 的小片段加载到寄存器
- 把 B 的小片段加载到寄存器
- 使用 FMA 指令连续更新寄存器中的 C 子块

这样做的好处：
- 减少对 C 的反复读写
- 最大化 FMA 吞吐
- 更适合手工向量化或编译器向量化

这就是为什么很多高性能库都有自己精心手写的 micro-kernel。

---

## 7. 第四层优化：Vectorization

现代 CPU 很强的一部分性能来自 SIMD。

比如：
- SSE
- AVX
- AVX2
- AVX-512
- ARM Neon
- SVE

### 7.1 向量化做了什么？
本来一次只算一个乘加，现在一次算多个。

例如 AVX2 的 256-bit 寄存器：
- 可以一次处理 8 个 `float`

AVX-512：
- 可以一次处理 16 个 `float`

### 7.2 为什么 GEMM 特别适合向量化？
因为它本质上是大量规则的乘加。

### 7.3 怎么让向量化更容易成功？
- 数据对齐更好
- 循环结构更规整
- 避免复杂分支
- 访问尽量连续
- blocking 后工作集更小更稳定

### 7.4 编译器自动向量化够吗？
有时够用，但高性能库通常不满足于“靠运气让编译器自动优化”。

真正冲极限性能时，经常会：
- 用 intrinsic
- 手写汇编
- 使用架构特定 micro-kernel

---

## 8. 第五层优化：Packing

这是很多新手容易忽略，但非常重要的一层。

### 8.1 packing 是什么？
把原始矩阵某个块拷贝到一个更适合计算的连续缓冲区里。

听起来像“额外拷贝会更慢”，但很多时候恰恰相反。

### 8.2 为什么 packing 有用？
因为原始矩阵布局不一定最适合 micro-kernel。

packing 可以带来：
- 更连续的访存
- 更容易对齐
- 更适合 SIMD load/store
- 更稳定的 cache 行为
- 更简单的内核逻辑

### 8.3 典型思路
- 把 A 的小块 pack 成内核喜欢的布局
- 把 B 的小块也 pack 成顺手的布局
- micro-kernel 针对 packed 数据猛烈计算

很多 BLAS 实现的强大之处，不只是算得快，而是：

> **它们把数据搬运和计算组织得非常讲究。**

---

## 9. 第六层优化：多线程并行

单核再抠也有上限，多核必须利用起来。

### 9.1 常见并行维度
可以按这些维度切工作：
- 按 `M` 方向切
- 按 `N` 方向切
- 按 block 切

### 9.2 并行时要注意什么？
- 线程负载均衡
- NUMA 影响
- false sharing
- cache 争用
- 带宽是否已经打满

### 9.3 不是线程越多越快
如果问题已经 memory-bound：
- 再加线程可能收益很小
- 甚至更慢

所以性能优化里一个常见真相是：

> **更多并行不一定更快，先看瓶颈在哪。**

---

## 10. CPU 上 GEMM 的典型优化路线

一个比较经典的路线是：

1. 先有正确的 baseline
2. 调整循环顺序
3. 做 cache blocking
4. 做 register blocking / micro-kernel
5. 向量化
6. packing
7. 多线程并行
8. 针对具体架构调 tile 参数

这也是很多 BLAS 库的基本套路，只是成熟库会做得更深、更狠、更细。

---

## 11. GPU 上 GEMM 为什么又是另一套思路？

GPU 当然也做 tiling，但重点会不一样。

### GPU 上常见关注点
- thread block / warp 组织
- global memory 访问是否 coalesced
- shared memory tiling
- register pressure
- occupancy
- Tensor Core 利用
- pipeline overlap

### CPU vs GPU 的一个粗暴对比

#### CPU 更强调
- cache hierarchy
- SIMD
- 分支与乱序执行
- 核间并行

#### GPU 更强调
- 海量线程隐藏延迟
- shared memory / global memory 协作
- warp 调度
- occupancy 与寄存器平衡

但核心思想还是同一类：

> **想办法提高数据复用，并尽可能喂满计算单元。**

---

## 12. 为什么数学库优化最后总落到 GEMM 上？

因为 GEMM 像一个“优化方法总汇”。

你在 GEMM 里能看到：
- 局部性优化
- 向量化
- 寄存器分块
- cache 分块
- packing
- 并行化
- 架构特化
- kernel 设计

很多其他算子优化思路，都能从 GEMM 里借鉴。

所以学 GEMM，不只是学一个算子，而是在学一整套性能优化思维。

---

## 13. 新手做 GEMM 优化最值得看的几个指标

做 benchmark 时建议至少观察：
- latency
- GFLOPS / TFLOPS
- cache miss
- memory bandwidth
- SIMD/FMA utilization
- 多线程扩展效率

如果有 profiler，还可以看：
- 指令吞吐
- stall 原因
- load/store 行为
- branch / vectorization 报告

---

## 14. 一个很实用的学习顺序

如果你要系统学 GEMM，我建议按这个顺序：

### 第 1 步：写 naive GEMM
先让自己真正看懂三层循环。

### 第 2 步：换循环顺序
对比 `ijk` 和 `ikj` 的差别。

### 第 3 步：加 blocking
感受 cache locality 是怎么改变性能的。

### 第 4 步：加 OpenMP 多线程
看单核和多核差异。

### 第 5 步：看编译器向量化报告
搞明白哪些循环被 vectorize 了，哪些没有。

### 第 6 步：尝试对比成熟库
比如：
- OpenBLAS
- BLIS
- MKL
- cuBLAS

你会立刻意识到工业级优化和“能跑”之间差了多远。

---

## 15. 一句话总结

GEMM 优化本质上就是：

> **通过 blocking、packing、vectorization、register reuse 和 parallelism，让数据复用更充分，让计算单元更忙，让内存拖后腿更少。**

如果你能把 GEMM 优化链条吃透，很多 HPC 和数学库优化问题你都会看得更明白。
