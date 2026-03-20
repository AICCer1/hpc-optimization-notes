# Profiling Practice

很多人学性能优化，最大的问题不是“不知道有哪些优化技巧”，而是：

> **不知道当前程序到底慢在哪里。**

这就是 profiling 的意义。

如果你不先看热点、不看瓶颈、不看访存、不看向量化，优化很容易变成拍脑袋。

这篇文档尽量不写成“工具菜单大全”，而是围绕你当前仓库里的 GEMM 例子，讲怎么做一套实战化分析。

---

## 1. 先记住一句最重要的话

> **先测，再猜；先 profile，再优化。**

别一上来就说：
- 这里肯定该加 OpenMP
- 这里肯定该上 SIMD
- 这里肯定是 cache miss

很多时候你一测，发现根本不是那么回事。

---

## 2. 什么时候你需要 profiling？

基本上只要出现下面这些情况，你就该 profile：

- 程序比预期慢很多
- 优化后没提升
- 加线程收益很差
- 改了 blocking 却没效果
- SIMD 没带来明显收益
- 不确定自己是 memory-bound 还是 compute-bound
- 想知道热点到底在哪个函数

如果你只是“感觉这里慢”，那还不够。

---

## 3. 你当前仓库里最适合拿来练手的例子

你现在已经有这些：

- `examples/gemm_baseline.cpp`
- `examples/gemm_blocked.cpp`
- `examples/gemm_openmp.cpp`

它们非常适合做 profiling 入门，因为：
- 逻辑简单
- 热点清晰
- 现象直观
- 容易对比

建议你拿这几个练：

### 第一阶段：看 baseline
观察不同循环顺序差异。

### 第二阶段：看 blocked
观察 blocking 是否改善 locality。

### 第三阶段：看 OpenMP
观察并行收益是否合理。

---

## 4. Profiling 大致分哪几类？

你可以把 profiling 粗分成四类。

### 4.1 时间热点分析
回答：
- 时间都花在哪个函数/代码段？

### 4.2 微架构分析
回答：
- cache miss 高不高？
- branch 有问题吗？
- IPC 高不高？
- stall 多不多？

### 4.3 向量化分析
回答：
- 编译器有没有成功 vectorize？
- 哪些循环没向量化？
- 原因是什么？

### 4.4 并行分析
回答：
- 线程扩展性怎么样？
- 有负载不均衡吗？
- 是否撞到带宽墙？

你做性能分析时，不一定要一次全做完，但脑子里最好知道这些层次。

---

## 5. 第一把刀：先做 benchmark，不然 profile 结果都没参照系

在你 profile 之前，先把 benchmark 跑起来。

比如：

```bash
cd examples
g++ -O3 -march=native -std=c++17 gemm_baseline.cpp -o gemm_baseline
./gemm_baseline 512 512 512 3
```

blocked 版本：

```bash
g++ -O3 -march=native -std=c++17 gemm_blocked.cpp -o gemm_blocked
./gemm_blocked 512 512 512 3 64 64 64
```

OpenMP 版本：

```bash
g++ -O3 -march=native -std=c++17 -fopenmp gemm_openmp.cpp -o gemm_openmp
OMP_NUM_THREADS=4 ./gemm_openmp 512 512 512 3
```

这一步的意义是：
- 先拿到性能数字
- 知道优化前后差多少
- 后面 profile 才有解释对象

---

## 6. Linux 上最朴素也最好用的工具：`perf`

如果你在 Linux 上，`perf` 是非常值得优先掌握的。

它的好处：
- 系统自带概率高
- 不用大型 IDE
- 能看热点，也能看硬件计数器
- 对 CPU 分析很实用

---

## 7. `perf stat`：先看全局指标

这是很好的第一步。

### 7.1 用法示例

```bash
perf stat ./gemm_baseline 512 512 512 3
```

或者：

```bash
perf stat ./gemm_blocked 512 512 512 3 64 64 64
```

### 7.2 你可以看什么
常见指标包括：
- cycles
- instructions
- IPC（instructions per cycle）
- cache-references
- cache-misses
- branch-misses
- task-clock

### 7.3 它适合回答什么问题
- 程序整体执行了多少指令？
- cache miss 是不是很离谱？
- IPC 高不高？
- blocked 版本是不是减少了某些开销？

### 7.4 怎么理解 IPC
粗暴理解：
- IPC 高，通常说明流水线吃得还不错
- IPC 很低，可能在等内存、等分支、等资源

当然别把它神化，它只是信号，不是最终结论。

---

## 8. `perf record` + `perf report`：看热点函数

### 8.1 用法示例

```bash
perf record ./gemm_baseline 1024 1024 1024 3
perf report
```

### 8.2 它适合干什么
回答：
- 时间主要花在哪些函数
- 是不是大部分时间都在 GEMM 内核
- 有没有意外热点

对于你这种教学例子，通常热点会非常集中。

这其实挺好，因为：
- 说明 benchmark 干净
- 你更容易盯住核心代码

---

## 9. `perf stat -d`：多看一点内存相关信息

可以试：

```bash
perf stat -d ./gemm_baseline 512 512 512 3
```

这会比默认统计多一些细节。

适合拿来做对比：
- baseline vs blocked
- serial vs OpenMP

如果 blocked 版本真的改善了 locality，某些缓存相关表现通常会更好一些。

不一定每台机器表现得都特别明显，但这个方向是对的。

---

## 10. 用编译器看向量化：别光猜“它应该被 vectorize 了”

很多人会误以为：
- 我开了 `-O3`
- 编译器肯定已经全帮我优化好了

现实经常不是这样。

### GCC 里可以看 vectorization report

例如：

```bash
g++ -O3 -march=native -fopt-info-vec-optimized -fopt-info-vec-missed gemm_baseline.cpp -o gemm_baseline
```

### 它能回答什么
- 哪些循环被成功向量化了
- 哪些循环没被向量化
- 可能是因为依赖、别名、控制流等原因失败

这一步非常重要，因为：

> **你以为自己在吃 SIMD，结果可能根本没吃上。**

---

## 11. OpenMP 场景怎么 profile？

OpenMP 版本最适合拿来做“线程扩展性”的观察。

### 11.1 先做最朴素实验

```bash
OMP_NUM_THREADS=1 ./gemm_openmp 1024 1024 1024 3
OMP_NUM_THREADS=2 ./gemm_openmp 1024 1024 1024 3
OMP_NUM_THREADS=4 ./gemm_openmp 1024 1024 1024 3
OMP_NUM_THREADS=8 ./gemm_openmp 1024 1024 1024 3
```

### 11.2 看什么
- 加速是不是近似线性
- 哪个线程数之后收益明显下降
- 是否早早撞到带宽墙

### 11.3 再结合 `perf stat`

```bash
OMP_NUM_THREADS=1 perf stat ./gemm_openmp 1024 1024 1024 3
OMP_NUM_THREADS=8 perf stat ./gemm_openmp 1024 1024 1024 3
```

可以对比：
- task-clock
- cycles
- instructions
- cache miss

如果线程加上去但收益有限，Roofline 那套判断就该进场了：
- 是不是 memory-bound？
- 是不是带宽快打满了？

---

## 12. 如果你有 VTune，CPU 分析会更爽

Intel VTune 是很强的 CPU profiling 工具。

它很适合做：
- hotspot analysis
- microarchitecture analysis
- memory access analysis
- threading analysis

如果你在 Intel 平台上做 HPC / math library 分析，它会很有帮助。

但新手阶段别因为没 VTune 就停住。

> `perf + benchmark + 编译器向量化报告` 已经够你迈出去很大一步了。

---

## 13. 如果你以后做 GPU，常用工具是什么？

虽然你当前代码主要还是 CPU 示例，但你脑子里最好先有这个地图。

### NVIDIA GPU 常见工具
- Nsight Systems
- Nsight Compute

### 它们分别更偏什么
- **Nsight Systems**：看整体时间线、kernel 调度、CPU/GPU 协作
- **Nsight Compute**：看单个 kernel 的详细性能指标

如果以后你写 CUDA / Triton / CUTLASS 相关例子，这两个会非常常用。

---

## 14. 做 profiling 时，最常见的错误姿势

### 错误 1：只跑一次就下结论
波动很常见，尤其小规模任务更明显。

### 错误 2：输入规模太小
规模太小，profiling 噪声很容易盖住真实特征。

### 错误 3：编译选项不一致
baseline 和 optimized 用不同编译参数对比，结论很容易歪。

### 错误 4：没固定线程数
OpenMP 场景下线程数不固定，结果很容易乱。

### 错误 5：只看运行时间，不看硬件计数器
你知道“变快了”，但不知道“为什么变快”。

### 错误 6：把 profiler 当裁判而不是线索提供者
profiler 给的是信号，不是上帝语录。你还得结合代码、算法、Roofline 一起判断。

---

## 15. 一个很实用的 profiling 工作流

推荐你按这个顺序来。

### 第一步：先 benchmark
拿到运行时间和 GFLOPS。

### 第二步：看 `perf stat`
先看全局：
- cycles
- instructions
- IPC
- cache-misses

### 第三步：看 `perf record/report`
确认热点是不是确实在目标内核。

### 第四步：看向量化报告
确认编译器到底有没有把你的热循环 vectorize。

### 第五步：结合 Roofline 判断
- 更像 memory-bound？
- 更像 compute-bound？

### 第六步：再决定优化动作
- locality / blocking / layout
- SIMD
- OpenMP
- packing
- micro-kernel

这套流程是比较稳的。

---

## 16. 用你仓库里的代码怎么做一个小练习？

这里给你一个很实际的小实验。

### 实验 A：baseline vs blocked
1. 编译两个版本
2. 跑相同输入规模
3. 记录时间和 GFLOPS
4. 用 `perf stat -d` 对比
5. 观察 blocked 是否改善 cache 行为

### 实验 B：OpenMP scaling
1. 固定输入规模
2. 改 `OMP_NUM_THREADS`
3. 看加速曲线
4. 观察什么时候收益开始掉
5. 结合 Roofline 思考是不是带宽先撞墙

### 实验 C：vectorization report
1. 用 GCC 打开向量化报告
2. 看哪个循环被 vectorize
3. 看 missed 的原因
4. 想办法把代码写得更容易向量化

这三个实验做完，你对“性能分析不是玄学”这件事就会有实感。

---

## 17. 一句话总结

profiling 的意义不是让你看一堆花里胡哨的数字，
而是帮你回答三个真正重要的问题：

1. **时间花在哪？**
2. **瓶颈像什么？**
3. **下一步最值得改哪里？**

如果这三个问题你能答出来，优化就开始从蒙眼乱挥，变成有方向地推进了。
