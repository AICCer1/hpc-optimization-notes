# Roofline Model

Roofline Model 是性能分析里一个特别好用的框架。

它的价值不在于“画图很酷”，而在于它能帮你快速判断一件很关键的事：

> **你的程序到底更像是被算力卡住，还是被带宽卡住。**

这个判断一旦错了，后面的优化很容易全走歪。

---

## 1. 为什么需要 Roofline Model？

很多人做性能优化时，容易掉进一种很常见的坑：
- 看到程序慢
- 就开始盲目优化
- 有时拼命做 SIMD
- 有时拼命加线程
- 有时拼命改循环

最后发现：
- 提升很有限
- 或者根本没提升

问题往往不是你不努力，而是：

> **你没有先判断瓶颈属于哪一类。**

Roofline Model 就是拿来干这个的。

---

## 2. 一句话理解 Roofline Model

Roofline Model 本质上是在说：

> 一个程序的可达到性能，通常受两个上限共同约束：
> 1. 机器的峰值算力
> 2. 机器的内存带宽

最终你能跑多快，取决于你先撞到哪堵墙。

所以它像一张“性能天花板图”。

---

## 3. 这张图上最重要的 3 个量

理解 Roofline，先把这三个量吃透。

### 3.1 Peak Compute Performance（峰值算力）
比如：
- CPU 理论峰值 GFLOPS / TFLOPS
- GPU 理论峰值 TFLOPS

它代表：

> **如果算力单元被完美喂满，理论上最多能算多快。**

这是上面那条“平顶”。

---

### 3.2 Memory Bandwidth（内存带宽）
比如：
- DDR 带宽
- HBM 带宽
- GPU global memory 带宽

它代表：

> **你从内存系统往计算单元运数据，最多能运多快。**

如果数据喂不进去，算力再强也白搭。

---

### 3.3 Arithmetic Intensity（算术强度）
这是 Roofline 最关键也最容易让新手发懵的概念。

它的定义可以粗暴记成：

> **每搬运 1 byte 数据，能做多少 floating-point operations（FLOPs）**

也就是：

```text
Arithmetic Intensity = FLOPs / Bytes moved
```

它反映的是：
- 你的计算“值不值”那次内存访问
- 数据复用高不高
- 你更偏算力型还是带宽型

---

## 4. Roofline 图怎么读？

虽然这里不画图，但你脑子里可以想象一张图：

- 横轴：Arithmetic Intensity
- 纵轴：Performance（GFLOPS / TFLOPS）

图上通常有两部分：

### 左下角斜线
这条线表示：

> **受内存带宽限制时，性能会随着 arithmetic intensity 提升而线性上升。**

这段区域一般叫：
- **memory-bound 区域**

因为数据复用不够高，程序主要在等数据。

---

### 上方平线
这条线表示：

> **一旦算术强度够高，性能就不再受带宽增长，而是被峰值算力封顶。**

这段区域一般叫：
- **compute-bound 区域**

因为这时候数据喂得已经差不多了，主要瓶颈变成算力单元本身。

---

## 5. 什么叫 memory-bound？

如果一个 kernel / 程序处于 memory-bound，意味着：

- 算力单元没吃满
- 主要问题是数据供给不够快
- 再做很多“纯算术优化”不一定有用

常见表现：
- 带宽占用高
- cache miss 高
- GFLOPS 上不去
- 加线程提升有限
- 指令单元还挺闲

### 对应优化方向
这时候更应该考虑：
- blocking / tiling
- 改数据布局
- 提高局部性
- 减少无效访存
- fusion
- prefetch
- shared memory / cache 重用

一句人话：

> **数据都喂不饱，先别急着磨刀，先想怎么把米运进厨房。**

---

## 6. 什么叫 compute-bound？

如果一个程序更接近 compute-bound，意味着：

- 数据供给已经不算主要问题
- 主要瓶颈是算力单元本身
- 你离理论峰值算力还有多远，就看计算内核写得有多狠

常见表现：
- arithmetic intensity 较高
- 带宽不是头号问题
- 更关注 FMA / Tensor Core / SIMD 利用率
- 更关注指令吞吐、流水线、寄存器复用

### 对应优化方向
这时候常考虑：
- vectorization
- instruction scheduling
- micro-kernel
- register blocking
- Tensor Core / MMA
- 减少 pipeline stall
- kernel 特化

一句人话：

> **饭已经端上来了，问题变成你嘴够不够快。**

---

## 7. Arithmetic Intensity 为什么这么重要？

因为它决定你更像在哪一边。

### 低 arithmetic intensity
说明：
- 每搬运一点数据，做的计算不多
- 数据价值榨得不够狠
- 更容易 memory-bound

### 高 arithmetic intensity
说明：
- 同样的数据能被反复利用
- 每次访存更“值钱”
- 更有机会逼近 compute roof

所以很多优化动作，本质上是在提高 arithmetic intensity。

比如：
- cache blocking
- shared memory tiling
- packing
- fusion

本质都是：

> **让同一份数据多算几次，少搬几次。**

---

## 8. 为什么 GEMM 经常被拿来做 Roofline 教材？

因为 GEMM 很适合展示“高算术强度”的价值。

### GEMM 的 FLOPs 很多
矩阵乘法里：
- 每个输出元素都要累加很多乘加
- 计算量很大

### 如果实现得好，数据复用也很强
通过：
- blocking
- packing
- register reuse

你能让 A 和 B 的数据被反复复用很多次。

这就让 GEMM 变成一个典型的：
- **有机会逼近 compute-bound** 的算子

这也是为什么高性能库经常能把 GEMM 做得非常接近硬件峰值。

---

## 9. 为什么有些算子很难摆脱 memory-bound？

并不是所有东西都像 GEMM 这么适合高复用。

例如：
- 简单逐元素操作
- 一些 sparse kernel
- 访存不规则的算法
- reduction 某些阶段
- scatter/gather 类型操作

这些任务往往：
- 数据复用差
- 访存不连续
- arithmetic intensity 低

所以它们常常更靠近带宽墙，而不是算力墙。

这时候你不能拿 GEMM 的思路硬套所有问题。

---

## 10. 怎么粗略估算 Arithmetic Intensity？

不需要一上来就做很学术的精确建模，粗估也很有价值。

### 例子：naive GEMM
假设矩阵乘法规模为：
- A: M x K
- B: K x N
- C: M x N

总 FLOPs 大约是：

```text
2 * M * N * K
```

如果从内存搬运的数据量粗略按：
- A: M*K
- B: K*N
- C: M*N

每个元素按 4 bytes（float）算，那么 bytes 大约是：

```text
4 * (M*K + K*N + M*N)
```

那么粗略 arithmetic intensity 是：

```text
AI ≈ (2*M*N*K) / (4*(M*K + K*N + M*N))
```

当然，真实情况会复杂很多，因为：
- cache 会重用
- 有些数据会多次加载
- 实现方式不同，实际 bytes moved 不同

但这个粗估已经足够帮助你建立直觉。

---

## 11. Blocking 为什么会让 Roofline 视角下的表现更好？

这是非常重要的一点。

Blocking 不会改变算法本身的理论 FLOPs，
但它可能显著改变：

- 实际 bytes moved
- cache 命中率
- 数据复用方式

于是从 Roofline 视角看，相当于：

> **你在减少“为了同样 FLOPs 需要搬的无效数据量”。**

这会把程序往更高有效 arithmetic intensity 的方向推。

所以 blocking 的收益，不只是“cache 好一点”，而是：

> **它让你更有机会远离 memory roof，向 compute roof 靠近。**

---

## 12. OpenMP 为什么有时加速明显，有时一般？

Roofline 也能解释这个问题。

### 情况 1：程序更偏 compute-bound
如果程序已经有比较高的算术强度，
那加线程更可能有效，因为：
- 计算单元有活可干
- 不是所有线程都在傻等数据

### 情况 2：程序更偏 memory-bound
如果程序主要卡在带宽：
- 线程一多
- 大家一起抢内存
- 带宽先打满
- 后面收益就会变差

所以你会看到很多情况：
- 1 线程到 2 线程提升明显
- 2 到 4 还有提升
- 8 到 16 就很一般
- 再往上甚至没啥赚头

这不是 OpenMP 不行，是你撞上了 Roofline 里的带宽墙。

---

## 13. 为什么“加 SIMD”不一定总有效？

这个也是一样的逻辑。

如果当前程序主要受内存带宽限制：
- 你把计算单元磨得再锋利
- 数据还是喂不过来
- 提升可能有限

但如果程序本来已经具有较高数据复用：
- SIMD 就很可能带来明显收益

所以别把 SIMD 神化成万能药。

它强，但要看是不是用对场景。

---

## 14. CPU 和 GPU 上 Roofline 怎么看？

原理一样，但具体观感会不一样。

### CPU 上
你更常把它和这些结合起来看：
- cache hierarchy
- SIMD
- 多核 scaling
- 内存带宽

### GPU 上
你更常把它和这些结合起来看：
- global memory bandwidth
- shared memory reuse
- Tensor Core 吞吐
- occupancy
- kernel fusion

### 本质不变
无论 CPU 还是 GPU，Roofline 都在帮你回答：

> **你当前离哪堵墙更近？**

---

## 15. 用你现在仓库里的例子怎么理解 Roofline？

你仓库里现在已经有这些例子：
- `gemm_baseline.cpp`
- `gemm_blocked.cpp`
- `gemm_openmp.cpp`

### `gemm_baseline.cpp`
适合看：
- 循环顺序改变如何影响数据访问模式
- 为什么同样算的是 GEMM，性能却能差不少

### `gemm_blocked.cpp`
适合看：
- blocking 如何改善局部性
- 为什么 tile size 会影响表现
- 为什么好的 blocking 能把程序往更高有效 AI 推

### `gemm_openmp.cpp`
适合看：
- 多线程是否有效
- 为什么线程数增加不一定线性加速
- 什么时候可能先撞带宽墙

也就是说，这几个例子虽然简单，但已经够拿来建立 Roofline 直觉了。

---

## 16. 实战里怎么用 Roofline，不要太学院派？

可以按这个朴素流程：

### 第一步：先估问题规模和工作特征
问自己：
- 计算量大吗？
- 数据搬运多吗？
- 访存规则吗？
- 数据能复用几次？

### 第二步：粗估 arithmetic intensity
不求绝对准，先建立直觉。

### 第三步：看 profiler / benchmark
观察：
- GFLOPS
- bandwidth
- cache miss
- scaling
- stall 原因

### 第四步：判断更像撞了哪堵墙
- 更像 memory-bound？
- 更像 compute-bound？

### 第五步：决定优化方向
- 如果 memory-bound：先盯 locality / layout / fusion / tiling
- 如果 compute-bound：先盯 SIMD / micro-kernel / instruction throughput

这个流程非常实用。

---

## 17. 新手最容易误解的点

### 误解 1：Roofline 能给出精确最终性能
不是。
它更像一个上界模型和分析框架。

### 误解 2：只要高 arithmetic intensity 就一定跑得快
也不一定。
还得看实现质量、向量化、并行、pipeline、内核设计。

### 误解 3：memory-bound 就没法优化
不是。
只是优化重点应该偏向访存和数据复用，而不是盲目堆算力技巧。

### 误解 4：compute-bound 就不用管内存
也不对。
很多高性能实现能接近 compute roof，恰恰是因为先把内存问题处理得够好了。

---

## 18. 一句话总结

Roofline Model 最有价值的地方在于：

> **它迫使你先判断“程序更受带宽限制还是更受算力限制”，再决定该从哪里下手优化。**

如果你没有这个判断框架，优化很容易变成玄学。

如果你有了这个框架，哪怕模型不精确，也已经比盲猜强太多。

---

## 19. 再压缩成一句最实用的话

如果你懒得记一整篇，只记这句：

> **Roofline 就是在帮你判断：你现在是在缺算力，还是缺喂数据的能力。**

这句话土，但真管用。
