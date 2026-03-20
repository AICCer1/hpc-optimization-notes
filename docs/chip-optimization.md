# Chip Optimization

这篇不是要把所有芯片优化细节一锅端，而是先给你一个比较实用的分析框架。

因为很多人一提“芯片性能优化”，脑子里就只剩下几个碎词：
- cache
- SIMD
- GPU
- Tensor Core
- 带宽

但真正做分析时，最重要的不是先背术语，而是先问：

> **当前这个程序，瓶颈到底在哪？**

这篇就是围绕这个问题展开。

---

## 1. 什么叫“芯片优化”？

你可以粗暴理解成：

> **让代码的执行方式更适合目标硬件的脾气。**

不同硬件的“脾气”不一样。

### CPU 更在意
- cache hierarchy
- SIMD
- 单核效率
- 多核调度
- 分支和流水线

### GPU 更在意
- 海量线程组织
- global memory / shared memory
- memory coalescing
- occupancy
- kernel 吞吐

### NPU / AI accelerator 更在意
- 特定数据流
- 特定算子模式
- tile / tensor layout
- 专用计算单元利用率

所以“芯片优化”从来不是一个固定动作，而是：

> **针对某类硬件，把程序组织得更对路。**

---

## 2. 一个很实用的分析框架

当你遇到性能问题时，建议先按下面这几步问自己。

### 2.1 是 compute-bound 还是 memory-bound？
这是第一问。

如果这个判断错了，后面优化方向很容易歪。

#### 更像 compute-bound
- 算很多
- 数据复用高
- 算力单元利用率是重点

#### 更像 memory-bound
- 访存很多
- 带宽紧张
- cache miss 多
- 算力单元在等数据

这也是为什么后面要看 Roofline。

---

### 2.2 热点函数在哪里？
第二问是：

> **时间到底花在哪？**

别凭感觉猜。
用 benchmark / profiler 看：
- 热点函数
- 热点循环
- hotspot 占比

有时候你以为慢在 GEMM，结果慢在数据准备；
有时候你以为慢在算子，结果慢在内存拷贝。

---

### 2.3 是算术组织有问题，还是数据组织有问题？
这一步特别关键。

#### 算术组织问题
比如：
- 向量化没吃上
- FMA 没吃上
- 并行划分不好
- Tensor Core 没用到

#### 数据组织问题
比如：
- 访存不连续
- layout 不对
- cache 局部性差
- shared memory 用得不值
- packing 缺失

很多性能问题，本质上不是“算得不够快”，而是“数据喂得太差”。

---

### 2.4 是单核问题，还是扩展性问题？
如果单线程已经慢，那先别急着上多线程。

如果单线程还行，但线程扩展很差，就要看：
- 是否带宽先打满
- 是否有同步开销
- 是否有负载不均衡
- 是否有 NUMA 问题

---

## 3. CPU 优化时常见要点

### 3.1 Cache locality
CPU 优化里最常见的一件事，就是让数据访问更“顺手”。

常用动作：
- blocking / tiling
- 调整循环顺序
- packing
- 改 layout

### 3.2 SIMD / 向量化
CPU 不吃 SIMD，很多时候就是白白浪费峰值。

常见动作：
- 写更规整的循环
- 减少复杂依赖
- 用 intrinsic
- 看编译器 vectorization report

### 3.3 多线程
多线程不是“开了就快”。

你要关注：
- 线程切分方式
- false sharing
- NUMA
- memory bandwidth 是否足够

### 3.4 Prefetch / pipeline
有些热点 kernel 会受益于更好的预取和流水线组织。

---

## 4. GPU 优化时常见要点

### 4.1 线程映射
先想清楚：
- 一个 block 算什么
- 一个 thread 负责什么
- 一个 warp 如何协作

### 4.2 内存访问模式
GPU 很怕访存组织乱。

常见问题：
- 不 coalesced
- bank conflict
- global memory 来回打太多

### 4.3 Shared memory
shared memory 很强，但不是摆着就赢。

你要问：
- 用它能不能显著提升数据复用？
- 会不会引入 bank conflict？
- 会不会把 occupancy 压太低？

### 4.4 Occupancy 和 register pressure
不是 occupancy 越高越好。

关键是平衡：
- 每线程寄存器够不够
- shared memory 占多少
- 同时在跑的 warp 够不够隐藏延迟

---

## 5. 数据布局为什么这么重要？

很多优化最后都会落回到 layout。

### 常见例子
- AoS vs SoA
- NHWC vs NCHW
- matrix row-major vs packed panel
- tensor tiling layout

布局决定的不是“代码好不好看”，而是：
- 连续访问能不能成立
- cache / shared memory 能不能高效利用
- SIMD / Tensor Core 好不好吃

很多时候你感觉是在优化 kernel，
实际上是在优化“数据怎么摆放”。

---

## 6. 一个工程上很有用的判断：先看收益最大的那一层

不要一上来就写花里胡哨的 intrinsic 或汇编。

更靠谱的顺序通常是：

### 第 1 层：先把算法/循环组织理顺
比如：
- loop reorder
- blocking
- tile size
- batch 策略

### 第 2 层：把数据组织理顺
比如：
- layout
- packing
- buffer reuse
- 减少中间结果落地

### 第 3 层：再去抠硬件特化
比如：
- SIMD intrinsic
- Tensor Core path
- micro-kernel
- 手写汇编

因为前两层往往更容易拿到大收益，
最后一层则更容易把自己写进沟里。

---

## 7. 常见优化动作，对应解决什么问题？

### Loop reorder
解决：
- 访存顺序不友好
- 局部性差

### Blocking / Tiling
解决：
- 工作集太大
- cache / shared memory 复用差

### Packing
解决：
- 原始布局不适合内核
- 连续访问差
- 向量化加载不顺

### OpenMP / 多线程
解决：
- 单核算不过来
- 任务可以分块并行

### SIMD
解决：
- 单核吞吐不够
- 向量单元闲置

### Fusion
解决：
- 中间结果反复落地
- 带宽浪费

### Layout transform
解决：
- 访存模式和硬件习惯不匹配

这样看就会清楚很多：

> **每个优化动作不是魔法，它是在解决特定问题。**

---

## 8. 新手最容易犯的错误

### 错误 1：先写复杂实现，再想它为什么快
顺序反了。
先判断瓶颈，再下手。

### 错误 2：把所有性能问题都当算力问题
实际上很多问题主要是访存问题。

### 错误 3：只看单点 benchmark，不看扩展性
单线程快，不代表多线程扩展也好。

### 错误 4：只看算法复杂度，不看硬件映射
理论复杂度一样，不代表硬件表现一样。

### 错误 5：不区分 CPU / GPU / NPU 的优化语言
不同硬件，优化重点差很多。

---

## 9. 怎么把这篇和仓库里别的内容串起来？

建议你按这个顺序看：

1. 先看这篇建立“分析框架”
2. 去看 `docs/gemm-optimization.md`
3. 跑 `examples/gemm_baseline.cpp`
4. 跑 `examples/gemm_blocked.cpp`
5. 看 `docs/roofline-model.md`
6. 看 `docs/profiling-practice.md`
7. 再回来看 `docs/cpu-vs-gpu-optimization.md`

这样你会发现：
- 芯片优化并不是散装技巧
- 它其实是一套从瓶颈判断到优化动作选择的逻辑链

---

## 10. 一句话总结

芯片优化最核心的不是“会多少招”，而是：

> **能不能先看出问题是算不满、喂不饱、排不好，还是布局不对。**

只要这个分析框架立住了，后面不管你看 CPU、GPU、NPU 还是数学库优化，脑子都会清楚很多。