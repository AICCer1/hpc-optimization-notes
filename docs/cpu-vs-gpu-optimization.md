# CPU vs GPU Optimization

这篇文档的目标不是喊口号式地说“GPU 更强”或者“CPU 更通用”，而是讲清楚：

> **为什么同样是优化一个算子，CPU 和 GPU 的思路会差这么多。**

如果你把这篇吃透，你再去看 GEMM、卷积、attention、FFT、reduction 这些算子，就更容易看懂不同平台上的优化套路。

---

## 1. 先讲结论：CPU 和 GPU 不是“谁更强”，而是“强的方式不一样”

### CPU 更擅长
- 低延迟
- 复杂控制流
- 分支较多的任务
- cache 友好的通用计算
- 单线程性能和灵活性
- 中小规模任务或不规则任务

### GPU 更擅长
- 大规模高吞吐并行
- 规则、密集、重复的数值计算
- 高计算密度 workload
- 可以拆成大量相似线程的任务
- 深度学习、dense matmul、图像/张量类计算

所以不要粗暴理解成：
- CPU = 落后
- GPU = 高级

更准确的是：

> **CPU 和 GPU 是两种风格完全不同的性能机器。**

---

## 2. 从硬件设计思路看本质差异

### 2.1 CPU 的设计倾向
CPU 更强调：
- 高单核性能
- 复杂控制能力
- 分支预测
- 乱序执行（Out-of-Order）
- 多级 cache
- 尽量降低单个任务的延迟

CPU 像什么？

> 像一个很聪明、反应快、临场处理能力强的老师傅。

它不一定一次喊来几万人干同样动作，但它处理复杂情况特别稳。

---

### 2.2 GPU 的设计倾向
GPU 更强调：
- 海量并行线程
- 高吞吐
- 用线程切换隐藏延迟
- 简化单线程控制能力，换更大的并行规模
- 针对规则计算设计的数据通路

GPU 像什么？

> 像一个特别擅长流水线化大规模重复劳动的超级工厂。

它不一定擅长每个工人单独思考，但非常擅长让一大群工人整齐划一狠狠干活。

---

## 3. CPU 优化最常盯什么？

做 CPU 优化时，常见关注点一般是这些：

### 3.1 Cache hierarchy
- L1 / L2 / L3 命中率
- cache miss
- 数据局部性
- blocking / tiling

### 3.2 SIMD / Vectorization
- AVX / AVX2 / AVX-512
- Neon / SVE
- FMA 利用率
- 编译器是否向量化成功

### 3.3 多核并行
- OpenMP / pthread / TBB
- 线程负载均衡
- false sharing
- NUMA

### 3.4 Pipeline / 指令级并行
- instruction throughput
- latency hiding（但方式和 GPU 不一样）
- load/store 压力

### 3.5 分支与控制流
- branch prediction
- 分支失败代价
- 复杂逻辑是否干扰流水线

一句话：

> **CPU 优化的很多核心，是尽量把 cache、SIMD 和多核都喂饱。**

---

## 4. GPU 优化最常盯什么？

做 GPU 优化时，关注点就明显变味了。

### 4.1 Thread / Warp / Block 组织
- 一个 kernel 怎么映射到线程
- 一个 block 多大
- 一个 warp 在干什么
- 线程划分是否合理

### 4.2 Memory hierarchy
- global memory
- shared memory
- register
- constant / texture（视平台而定）

### 4.3 Memory access pattern
- memory coalescing
- bank conflict
- 非连续访存
- 数据搬运是否高效

### 4.4 Occupancy
- 活跃 warp 数量
- 是否有足够线程隐藏内存延迟
- 寄存器占用太高会不会压低 occupancy

### 4.5 Kernel 结构
- shared memory tiling
- double buffering / pipeline
- Tensor Core 使用
- kernel fusion

一句话：

> **GPU 优化的很多核心，是让海量线程组织得更合理，让访存更顺，让吞吐更高。**

---

## 5. CPU 和 GPU 都在解决“延迟”和“带宽”问题，但方法完全不同

### CPU 怎么隐藏延迟？
- 靠 cache
- 靠乱序执行
- 靠预取
- 靠 SIMD
- 靠分支预测

### GPU 怎么隐藏延迟？
- 靠更多线程切换
- 靠 warp 调度
- 靠 occupancy
- 靠 shared memory 缓存热点数据

所以一个很关键的认识是：

> **CPU 靠“聪明”隐藏延迟，GPU 靠“人多”隐藏延迟。**

虽然这话不严谨，但非常好记。

---

## 6. memory-bound 和 compute-bound：两边都讲，但含义体验不完全一样

### 6.1 在 CPU 上看 memory-bound
如果 CPU 程序 memory-bound，常见表现是：
- 算术单元吃不饱
- cache miss 很高
- 大量时间耗在等数据
- 加线程不一定有用

这时常见优化动作是：
- blocking
- 改数据布局
- 减少 cache miss
- 提高空间/时间局部性
- 向量化提升单位访存收益

### 6.2 在 GPU 上看 memory-bound
如果 GPU kernel memory-bound，常见表现是：
- global memory 成为主瓶颈
- SM 里算力没吃满
- 带宽已经打得差不多了
- kernel 再多做点线程也没明显提升

这时常见动作是：
- shared memory tiling
- memory coalescing
- fusion 减少中间结果回写
- 改 layout
- 减少无效访存

所以虽然两边都说 memory-bound，但你真正下手的“工程动作”并不一样。

---

## 7. GEMM：为什么 CPU 和 GPU 优化套路差这么多？

GEMM 是最适合用来做对比的例子。

### 7.1 CPU 上 GEMM 典型思路
- 选合适循环顺序
- cache blocking
- register blocking
- micro-kernel
- SIMD / FMA
- packing
- 多线程并行

CPU 上最强调的是：
- cache hierarchy
- 寄存器复用
- 指令吞吐

### 7.2 GPU 上 GEMM 典型思路
- threadblock tiling
- warp-level 分工
- shared memory 缓冲 A/B tile
- register fragment 累加
- Tensor Core / MMA
- pipeline overlap
- occupancy 与 register pressure 平衡

GPU 上最强调的是：
- 大规模线程组织
- memory coalescing
- shared memory 重用
- 吞吐路径设计

### 7.3 核心差异
CPU 的思路更像：
> “把一个聪明工人和少量助手的效率榨到极致。”

GPU 的思路更像：
> “设计一个极其高效的车间调度，让成千上万人同时干。”

---

## 8. 卷积、attention、reduction 为什么也会分出不同套路？

因为它们都绕不开平台特性。

### CPU 上
- 更看重 cache locality
- 更看重分块和向量化
- 更看重线程同步成本
- 对复杂边界情况更能处理

### GPU 上
- 更看重 kernel 是否规则
- 更看重 batch 化和并行映射
- 更看重 memory access pattern
- 更看重 fusion 和中间张量减少落地

所以很多 AI 算子会出现这种现象：
- CPU 版重点是 cache + vector + threads
- GPU 版重点是 tiling + shared memory + occupancy + fusion

---

## 9. 为什么 CPU 常提 AoS vs SoA，而 GPU 更强调 coalescing？

### 在 CPU 上
AoS（Array of Structures）和 SoA（Structure of Arrays）影响的是：
- cache line 利用率
- 向量化难易
- prefetch 效果

### 在 GPU 上
数据布局同样重要，但你会更直接遇到：
- 同一个 warp 里的线程是不是连续访问
- global memory transaction 是否高效
- shared memory 是否 bank conflict

所以 GPU 上“布局问题”常常会更赤裸裸地体现在吞吐里。

---

## 10. CPU 优化里常见的杀手锏

### 10.1 Blocking / Tiling
让 cache 更友好。

### 10.2 Vectorization
利用宽寄存器一次算多个元素。

### 10.3 Prefetch
提前搬数据。

### 10.4 Thread affinity / NUMA awareness
多路机器上很关键。

### 10.5 Micro-kernel
针对架构手工设计极致热点内核。

---

## 11. GPU 优化里常见的杀手锏

### 11.1 Shared memory tiling
减少 global memory 访问。

### 11.2 Memory coalescing
让线程束访问尽量连续。

### 11.3 Kernel fusion
减少中间张量来回落地。

### 11.4 Tensor Core / 专用指令路径
直接吃硬件专用吞吐。

### 11.5 Occupancy tuning
在寄存器、shared memory、并发度之间找平衡。

---

## 12. 什么时候优先做 CPU 优化，什么时候直接上 GPU？

这是很实际的问题。

### 更适合优先做 CPU 优化的情况
- 任务规模不大
- 控制流复杂
- 数据访问不规则
- 数据搬到 GPU 的成本不划算
- 部署环境没有 GPU
- 延迟要求非常敏感，且 batch 很小

### 更适合优先上 GPU 的情况
- 计算非常密集
- 问题规模大
- 数据天然可并行
- 访问模式相对规则
- 已经在 GPU 上有完整 pipeline
- AI / dense tensor workload 明显吃 GPU

一句大白话：

> **数据够大、计算够密、并行够多，GPU 就值钱；否则 CPU 往往更省事。**

---

## 13. 新手最容易踩的几个误区

### 误区 1：GPU 一定比 CPU 快
不一定。
小任务、低 batch、复杂分支、强依赖控制流的任务，CPU 可能更好。

### 误区 2：CPU 优化就是开 O3
不是。
O3 很重要，但离真正的高性能还差很远。

### 误区 3：GPU 优化就是多开线程
不是。
线程多不代表访存就对，也不代表 occupancy 就合理。

### 误区 4：只看 FLOPS 不看带宽
会死得很惨。
很多程序根本不是算力瓶颈，而是 memory-bound。

### 误区 5：CPU 和 GPU 的好优化可以直接照搬
不行。
原则可能相通，但落地方式往往完全不同。

---

## 14. 你做性能分析时，两边应该怎么想？

### CPU 侧建议先问
1. cache miss 高吗？
2. 是否被 vectorize 了？
3. 核心热点在哪里？
4. 是否线程扩展正常？
5. NUMA / false sharing 有没有问题？

### GPU 侧建议先问
1. global memory 是否是瓶颈？
2. 是否 coalesced？
3. shared memory 用得值不值？
4. occupancy 是否受限？
5. register pressure 是否太高？
6. kernel launch / fusion 是否合理？

---

## 15. 对工程实践最有用的一句话

如果你只记一条，请记这个：

> **CPU 优化更多是在“提高单个核心和少量核心的效率”，GPU 优化更多是在“设计大规模并行协作的高吞吐执行方式”。**

这句话不够严谨，但很适合拿来当脑子里的路标。

---

## 16. 一个很实用的学习顺序

如果你想把 CPU / GPU 对比真正学懂，我建议：

### 第一步：先把 CPU GEMM 优化走一遍
理解：
- loop reorder
- blocking
- vectorization
- threading

### 第二步：再看 GPU GEMM
理解：
- threadblock
- warp
- shared memory
- occupancy

### 第三步：对照同一个算子比较两边思路
比如 GEMM 或 convolution。

### 第四步：再看 profiler
- CPU：perf / VTune / compiler vector report
- GPU：nsight systems / nsight compute

到这一步，你对性能优化的理解就会开始从“背名词”变成“能判断问题在哪”。

---

## 17. 一句话总结

CPU 和 GPU 优化的共同目标，都是：
- 提高硬件利用率
- 减少无效访存
- 提高吞吐或降低延迟

但它们的世界观不一样：

- **CPU 世界观**：cache、SIMD、多核、低延迟、灵活控制
- **GPU 世界观**：海量线程、吞吐、shared memory、coalescing、occupancy

搞懂这个差异，你看 HPC / AI kernel / 数学库优化时，脑子就不会老打结。