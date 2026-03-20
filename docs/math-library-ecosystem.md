# Math Library Ecosystem Map

这份文档的目标不是列百科全书，而是帮你快速建立一个“地图感”：

- 这些数学库各自是干嘛的
- 它们更偏 CPU 还是 GPU
- 它们是通用库、内核库、还是编译/生成框架
- 什么时候该用哪个

---

## 1. 先分几大类

可以粗分成这几层：

### 1.1 基础线性代数库
代表：
- BLAS
- LAPACK

它们更像“接口标准 / 经典基础能力”。

### 1.2 CPU 高性能实现
代表：
- OpenBLAS
- BLIS
- Intel MKL / oneMKL
- Eigen（部分场景）

### 1.3 GPU 高性能实现
代表：
- cuBLAS
- cuBLASLt
- CUTLASS
- rocBLAS
- hipBLAS

### 1.4 深度学习 / 张量计算优化库
代表：
- oneDNN
- cuDNN
- MIOpen
- ACL（Arm Compute Library）

### 1.5 编译 / 自动优化 / Kernel 生成框架
代表：
- TVM
- Triton
- MLIR 相关栈
- Halide（更广义）

### 1.6 FFT / Sparse / 特定领域库
代表：
- FFTW
- cuFFT
- MKL FFT
- SuiteSparse
- Intel oneMKL Sparse
- cuSPARSE

---

## 2. BLAS 是什么，为什么总绕不开？

BLAS = Basic Linear Algebra Subprograms。

它定义了一套非常经典的线性代数操作接口。

### BLAS 经典分层
- **Level 1**：向量-向量
- **Level 2**：矩阵-向量
- **Level 3**：矩阵-矩阵

其中最重要的通常是：
- **GEMM**（矩阵乘）

因为 Level 3 操作计算密度高，更容易把硬件吃满。

所以很多性能优化的“主战场”都在 BLAS Level 3。

---

## 3. OpenBLAS

### 它是什么
一个开源的高性能 BLAS/LAPACK 实现。

### 它适合什么
- 通用 CPU 场景
- 想快速拿到不错性能
- 不想被商业生态强绑定

### 它的特点
- 开源
- 应用广泛
- 在很多 CPU 平台上都有支持
- 很多 Python 科学计算栈会间接用到它

### 什么时候用它
- 你需要一个稳妥的 CPU BLAS 后端
- 想做基线对比
- 想研究开源实现思路

---

## 4. BLIS

### 它是什么
一个非常有教学价值、结构也很清晰的高性能 BLAS 框架/实现。

### 它为什么值得看
很多人学 GEMM 优化，会很喜欢看 BLIS 的思路，因为它对：
- packing
- micro-kernel
- 分层 blocking
- 架构适配

这些概念讲得非常“工程化”。

### 它适合什么
- 学习 GEMM / BLAS 的结构化设计
- 做内核和分层优化研究
- 理解高性能库是怎么组织的

### 一句话评价
如果 OpenBLAS 更像“现成能打”，那 BLIS 往往更像“适合拆开研究为什么能打”。

---

## 5. MKL / oneMKL

### 它是什么
Intel 生态里非常重要的高性能数学库。

### 它常见包含
- BLAS
- LAPACK
- FFT
- Sparse
- 各种数值计算相关能力

### 它的特点
- 在 Intel CPU 上通常很强
- 工程成熟度高
- 在工业界和科研里都很常见

### 什么时候适合用
- 你主要跑在 Intel CPU 上
- 你追求成熟、稳、快
- 你想快速拿到很强的基准性能

### 注意点
- 生态和平台绑定感会更强一些
- 如果你要跨厂商或更开放研究，可能会同时对比 OpenBLAS / BLIS

---

## 6. oneDNN

### 它是什么
一个更偏深度学习和张量计算优化的库。

### 它擅长什么
- convolution
- matmul
- normalization
- activation
- quantization
- 深度学习相关 primitive

### 它和 BLAS 的区别
BLAS 更像传统线性代数基建。
oneDNN 更像为深度学习 workload 深度优化过的一层。

### 什么时候适合用
- 你关心 AI 推理/训练里的算子性能
- 你想研究 CPU 上深度学习内核优化
- 你在看 oneAPI / Intel AI software stack

---

## 7. Eigen

### 它是什么
一个 C++ 模板库，在线性代数和小中型矩阵运算里非常常见。

### 它的特点
- header-only，用起来方便
- API 友好
- 工程接入成本低
- 对表达式模板做得很多

### 它适合什么
- C++ 工程里快速使用矩阵/向量操作
- 小中规模问题
- 对开发体验比较敏感的场景

### 它不一定最适合什么
- 极限 HPC 调优场景
- 要硬刚最顶级 BLAS/GEMM 峰值性能的时候

---

## 8. cuBLAS / cuBLASLt

### 它是什么
NVIDIA GPU 生态里的核心线性代数库。

### 它擅长什么
- GPU 上的 GEMM / matmul
- 高吞吐线性代数运算
- Tensor Core 相关高性能路径

### cuBLASLt 为什么值得单独提
因为它相比传统 cuBLAS：
- 更灵活
- 更适合复杂 matmul 配置
- 更强调 autotuning / layout / epilogue 等能力

### 什么时候适合用
- 你在 NVIDIA GPU 上做高性能矩阵运算
- 你要对标 GPU GEMM 基准
- 你在做 AI 相关 dense kernel 优化

---

## 9. CUTLASS

### 它是什么
NVIDIA 提供的 CUDA C++ 模板库，用来构建高性能 GEMM 和相关 kernel。

### 为什么它很重要
因为它不只是“现成函数库”，更像是：

> **一个理解 GPU GEMM 内核组织方式的教材 + 工具箱**

### 你能从它学到什么
- threadblock tiling
- warp-level MMA
- shared memory pipeline
- Tensor Core kernel 结构
- epilogue 设计

### 它适合什么
- 学 GPU GEMM 优化
- 定制 kernel
- 想在 cuBLAS 之外研究更底层实现

---

## 10. FFTW / cuFFT

### FFTW
CPU 上非常经典的 FFT 库。

它的特点：
- 历史悠久
- 非常强
- 对 FFT 规划和执行有很多优化经验积累

### cuFFT
NVIDIA GPU 上的 FFT 库。

它适合：
- GPU 上的频域计算
- 信号处理、科学计算相关 workload

### 为什么 FFT 重要
因为它代表了另一类典型优化问题：
- 不只是 dense matmul
- 数据访问模式更复杂
- 重排 / butterfly / cache 行为都很关键

---

## 11. Sparse 相关库

dense 世界和 sparse 世界是两套气质。

### 常见 sparse 库
- SuiteSparse
- oneMKL Sparse
- cuSPARSE

### sparse 的难点
- 不规则访存
- 负载不均衡
- 向量化困难
- 带宽瓶颈更明显

所以很多 dense GEMM 那套方法，并不能无脑平移到 sparse。

---

## 12. TVM / Triton / 编译优化框架

这类工具和传统“数学库”不完全一样。

### TVM
更像编译栈 / 自动优化框架。

特点：
- 从更高层描述算子
- 自动 schedule / autotune
- 支持多种硬件后端

### Triton
更像面向 GPU kernel 的高层编程模型。

特点：
- 比 CUDA 更高层
- 写定制 matmul / attention / fused kernel 很流行
- 在 AI kernel 场景里很强

### 它们和 BLAS 库有什么区别？
BLAS / cuBLAS 这类库更像“现成高手”。
TVM / Triton 更像“帮你生成高手动作的工具”。

### 什么时候适合用
- 你需要定制化 kernel
- 标准库不够贴需求
- 你想做自动优化/编译研究
- 你在 AI 编译器/算子生成方向工作

---

## 13. 怎么选库？一个很实用的判断框架

你可以按这几个问题来选：

### 13.1 你跑在哪种硬件上？
- Intel CPU
- AMD CPU
- ARM CPU
- NVIDIA GPU
- AMD GPU
- NPU / AI accelerator

硬件决定了大半路线。

### 13.2 你是想“直接用”，还是想“研究底层”？
- 直接用：更偏 MKL、OpenBLAS、cuBLAS、oneDNN
- 研究底层：更适合 BLIS、CUTLASS、Triton、TVM

### 13.3 你要优化的 workload 是什么？
- dense linear algebra
- deep learning primitives
- FFT
- sparse
- fused operators

不同 workload 最强工具完全可能不同。

### 13.4 你更在乎什么？
- 最终性能
- 可移植性
- 易用性
- 可研究性
- 可定制性

很多时候这是 trade-off，不可能全都要。

---

## 14. 一张粗暴但实用的定位表

### CPU 传统线性代数
- OpenBLAS：开源通用
- BLIS：结构清晰，适合研究
- MKL / oneMKL：工业强者，Intel 生态强

### CPU 深度学习算子
- oneDNN：非常重要
- Eigen：工程友好，但不是所有场景都追极限

### NVIDIA GPU dense 计算
- cuBLAS / cuBLASLt：工业主力
- CUTLASS：适合学习和定制
- Triton：适合快速开发定制高性能 kernel

### FFT
- FFTW：CPU 经典
- cuFFT：NVIDIA GPU 经典

### Sparse
- SuiteSparse：CPU 重要生态
- cuSPARSE：GPU 常用
- oneMKL Sparse：Intel 生态

### 自动优化 / 编译路线
- TVM
- Triton
- MLIR 生态

---

## 15. 学习顺序建议

如果你是按“先建立全局感，再深入”来学，我建议：

### 第一层：先理解 BLAS / GEMM
因为这是 dense 计算主轴。

### 第二层：看 CPU 经典实现
- OpenBLAS
- BLIS
- MKL

### 第三层：看 GPU 路线
- cuBLAS
- CUTLASS
- Triton

### 第四层：再扩展到 FFT / sparse / compiler
这样脑子里不会乱成一锅粥。

---

## 16. 一句话总结

这些数学库和优化框架，本质上可以理解成三类角色：

1. **接口/标准**：比如 BLAS、LAPACK
2. **高性能实现**：比如 OpenBLAS、MKL、cuBLAS、oneDNN
3. **生成/优化框架**：比如 TVM、Triton、CUTLASS（半工具箱半实现）

如果你把这个角色关系想清楚，再看 HPC / 数学库生态，就不会老觉得名词满天飞却抓不住主线。
