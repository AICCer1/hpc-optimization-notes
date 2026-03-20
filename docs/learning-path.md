# Learning Path

如果你第一次打开这个仓库，最容易遇到的问题不是“内容太少”，而是：

> **东西已经有一些了，但不知道该先看哪个。**

这份文档就是拿来解决这个问题的。

我把仓库里的内容按学习路线串起来了。你不用从头到尾全啃，可以按你当前阶段选一条走。

---

## 0. 先怎么用这个仓库？

你可以把这个仓库当成三层结构：

### 第一层：建立概念
先知道：
- HPC 优化到底在优化什么
- CPU / GPU 各自的性能逻辑是什么
- 为什么有些优化有效，有些没效果

### 第二层：建立分析框架
也就是：
- 什么叫 memory-bound / compute-bound
- Roofline 怎么帮助判断瓶颈
- profiling 怎么帮助定位热点

### 第三层：建立代码直觉
通过 GEMM 例子去看：
- 循环顺序
- blocking
- OpenMP
- SIMD
- packing

所以这个仓库不是纯文档，也不是纯代码，而是：

> **文档讲方法，代码给你手感。**

---

## 1. 最推荐的新手路线

如果你是第一次系统碰这些内容，建议按下面顺序：

### 第一步：先看 HPC 基础
- [`docs/hpc-basics.md`](./hpc-basics.md)

为什么先看这个？
因为你得先知道：
- cache
- SIMD
- bandwidth
- parallelism

这些词到底在说啥。

---

### 第二步：看芯片优化大图
- [`docs/chip-optimization.md`](./chip-optimization.md)

这一篇比较像“问题分析框架”。

你会先建立一个基本习惯：
- 先问瓶颈在哪
- 再问该怎么优化

而不是一上来就乱抡优化术语。

---

### 第三步：看 GEMM 优化专题
- [`docs/gemm-optimization.md`](./gemm-optimization.md)

这是整个仓库最值得反复看的核心文档之一。

为什么？
因为 GEMM 基本把这些关键思想都串起来了：
- loop reorder
- blocking
- register reuse
- vectorization
- packing
- threading

你把这篇吃透，很多 HPC / 数学库优化问题都会更容易看明白。

---

### 第四步：看 CPU vs GPU 优化差异
- [`docs/cpu-vs-gpu-optimization.md`](./cpu-vs-gpu-optimization.md)

这一篇主要解决：

> “为什么同样是优化一个算子，CPU 和 GPU 的思路会差这么多？”

如果你后面打算碰 CUDA / Triton / AI kernel，这篇很有必要。

---

### 第五步：看 Roofline Model
- [`docs/roofline-model.md`](./roofline-model.md)

这一篇是性能分析的判断框架。

它帮你回答：
- 是算力不够？
- 还是数据喂不进去？

这一步很关键，因为很多人会优化失败，不是因为不会技巧，而是因为瓶颈判断错了。

---

### 第六步：看 profiling 实战
- [`docs/profiling-practice.md`](./profiling-practice.md)

这篇会把前面的概念往实战里落：
- benchmark 怎么做
- `perf` 怎么看
- vectorization report 怎么看
- OpenMP scaling 怎么观察

可以理解成：

> **前面是在学“脑子”，这篇是在学“怎么动手查证”。**

---

### 第七步：看数学库生态地图
- [`docs/math-library-ecosystem.md`](./math-library-ecosystem.md)
- [`docs/math-library-optimization.md`](./math-library-optimization.md)

这个阶段更适合你已经有一点基础以后再看。

因为这里会开始涉及：
- OpenBLAS
- BLIS
- MKL
- oneDNN
- cuBLAS
- CUTLASS
- Triton
- TVM

这时候你再看，就不会只觉得名词多，而能大概知道它们各自的定位。

---

## 2. 对应代码应该怎么配合看？

建议不要“文档全看完再跑代码”，那样容易发虚。

更好的方式是：

### 看完 GEMM 文档后，跑这些

#### 1）循环顺序
- [`examples/gemm_baseline.cpp`](../examples/gemm_baseline.cpp)

重点看：
- `ijk`
- `ikj`

你会直观感受到：
- 只是循环顺序变化，性能就会不同

---

#### 2）blocking
- [`examples/gemm_blocked.cpp`](../examples/gemm_blocked.cpp)

重点看：
- `BM / BN / BK`
- 为什么 tile size 会影响表现

---

#### 3）OpenMP 并行
- [`examples/gemm_openmp.cpp`](../examples/gemm_openmp.cpp)

重点看：
- 线程数变化
- 加速是否线性
- 什么时候收益开始变差

---

#### 4）SIMD
- [`examples/gemm_simd.cpp`](../examples/gemm_simd.cpp)

重点看：
- AVX intrinsic 长什么样
- scalar 和 SIMD 为什么会有性能差异

---

#### 5）packing
- [`examples/gemm_packed.cpp`](../examples/gemm_packed.cpp)

重点看：
- 为什么先 pack 再算
- 为什么“额外拷贝”不一定更慢

---

## 3. 如果你只想先建立“最短学习闭环”

那我建议直接走这个 5 步：

### 路线 A：最快上手版
1. [`docs/gemm-optimization.md`](./gemm-optimization.md)
2. [`examples/gemm_baseline.cpp`](../examples/gemm_baseline.cpp)
3. [`examples/gemm_blocked.cpp`](../examples/gemm_blocked.cpp)
4. [`docs/roofline-model.md`](./roofline-model.md)
5. [`docs/profiling-practice.md`](./profiling-practice.md)

这条路的优点是：
- 不绕弯
- 很快就能形成“概念 + 代码 + 分析”的闭环

---

## 4. 如果你更偏工程实践

那我建议走这条：

### 路线 B：工程分析版
1. [`docs/chip-optimization.md`](./chip-optimization.md)
2. [`docs/cpu-vs-gpu-optimization.md`](./cpu-vs-gpu-optimization.md)
3. [`docs/roofline-model.md`](./roofline-model.md)
4. [`docs/profiling-practice.md`](./profiling-practice.md)
5. [`examples/gemm_openmp.cpp`](../examples/gemm_openmp.cpp)
6. [`examples/gemm_simd.cpp`](../examples/gemm_simd.cpp)
7. [`examples/gemm_packed.cpp`](../examples/gemm_packed.cpp)

这条路更适合：
- 你已经会写点代码
- 现在更想理解“怎么分析、怎么判断、怎么选优化方向”

---

## 5. 如果你更偏数学库 / AI kernel 方向

建议走这条：

### 路线 C：库与算子版
1. [`docs/gemm-optimization.md`](./gemm-optimization.md)
2. [`docs/math-library-optimization.md`](./math-library-optimization.md)
3. [`docs/math-library-ecosystem.md`](./math-library-ecosystem.md)
4. [`docs/cpu-vs-gpu-optimization.md`](./cpu-vs-gpu-optimization.md)
5. [`docs/roofline-model.md`](./roofline-model.md)

这条路比较适合：
- 想往 BLAS / kernel / AI 编译器方向靠
- 想建立“库、硬件、算子”三者之间的关系感

---

## 6. 一个很实际的建议：别一口气全看完

这个仓库的正确打开方式不是：
- 今天把所有 md 文件全部通读一遍

更好的方式是：
- 看一篇文档
- 跑一个例子
- 改一个参数
- 再回来复盘

比如：
1. 先看 `gemm-optimization.md`
2. 跑 `gemm_baseline.cpp`
3. 再跑 `gemm_blocked.cpp`
4. 看 `roofline-model.md`
5. 再用 `perf` 看一次

这种学习方式比纯看文档强很多。

---

## 7. 如果你现在就问“我先看哪个？”

我直接给你一个最短答案：

### 最推荐起点
先看：
- [`docs/gemm-optimization.md`](./gemm-optimization.md)

然后配合：
- [`examples/gemm_baseline.cpp`](../examples/gemm_baseline.cpp)
- [`examples/gemm_blocked.cpp`](../examples/gemm_blocked.cpp)

再接着看：
- [`docs/roofline-model.md`](./roofline-model.md)
- [`docs/profiling-practice.md`](./profiling-practice.md)

如果你按这个顺序走，基本不会迷路。

---

## 8. 一句话总结

这个仓库最适合的打开方式是：

> **先用 GEMM 建立性能优化直觉，再用 Roofline 和 profiling 建立分析框架，最后再扩展到 CPU/GPU 差异和数学库生态。**

别一上来想把整个 HPC 世界一口吞了，容易把自己噎着。