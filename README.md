# hpc-optimization-notes

一个面向 **HPC、芯片性能优化、数学库优化** 的资料仓库。

目标：
- 系统整理高性能计算相关知识
- 积累 CPU / GPU / NPU / AI 芯片优化经验
- 沉淀 BLAS / GEMM / FFT / 并行计算 / 向量化 / 内存优化相关内容
- 做成一个既能学习，也能慢慢沉淀工程经验的仓库

---

## 仓库方向

### 1. HPC 基础
- Roofline Model
- 内存层次结构
- Cache / TLB / NUMA
- 并行计算基础
- 向量化 / SIMD / FMA
- 带宽 vs 算力瓶颈

### 2. 芯片性能优化
- CPU 性能分析
- GPU Kernel 优化
- NPU / AI Accelerator 优化
- Memory-bound vs Compute-bound 分析
- 数据布局优化
- 指令级并行 / 流水线 / 访存对齐

### 3. 数学库优化
- BLAS
- GEMM
- FFT
- Sparse 算子
- Reduction / Scan
- 卷积与张量算子优化
- 算子 fusion / tiling / blocking / prefetch

### 4. Benchmark 与 Profiling
- microbenchmark
- profiler 使用
- 性能瓶颈定位
- benchmark 结果记录模板

---

## 目录结构

```text
hpc-optimization-notes/
├─ README.md
├─ docs/
│  ├─ roadmap.md
│  ├─ hpc-basics.md
│  ├─ chip-optimization.md
│  └─ math-library-optimization.md
├─ benchmarks/
│  └─ benchmark-template.md
├─ examples/
│  └─ README.md
└─ notes/
   └─ glossary.md
```

---

## 适合沉淀的主题

### HPC 基础专题
- 算力模型与性能上界
- latency / throughput 区别
- cache miss 对性能的影响
- 多线程扩展性
- Amdahl's Law / Gustafson's Law

### 芯片优化专题
- CPU 上做 GEMM 为什么要 blocking
- GPU 上为什么要做 shared memory tiling
- 向量寄存器怎么影响吞吐
- memory coalescing / bank conflict 是什么
- 哪些问题适合算子 fusion

### 数学库专题
- BLAS 分层
- GEMM 是为什么这么重要
- MKL / OpenBLAS / BLIS / cuBLAS 的差异
- oneDNN / CUTLASS / TVM / Triton 的定位
- 如何设计 benchmark 比较不同实现

---

## 你后续可以怎么用这个仓库

1. 当学习笔记仓库
2. 当性能分析案例仓库
3. 当 benchmark 结果沉淀仓库
4. 当面试 / 研究 / 工程项目的知识底座

---

## 下一步建议

你可以继续让我补这些内容：
- 一份 **系统学习路线图**
- 一份 **GEMM 优化专题**
- 一份 **CPU vs GPU 优化方法对比**
- 一份 **数学库生态地图**
- benchmark 示例代码（C++ / Python）
