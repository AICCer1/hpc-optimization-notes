# Examples

这里放一些可跑的小例子，用来帮助理解性能优化，不追求一上来就工业级。

## 当前内容

### 1. `gemm_baseline.cpp`
一个最基础的 GEMM benchmark，对比：
- `ijk` 循环顺序
- `ikj` 循环顺序

适合新手观察“只是循环顺序变化，性能就可能不一样”。

编译示例：

```bash
g++ -O3 -march=native -std=c++17 gemm_baseline.cpp -o gemm_baseline
./gemm_baseline 512 512 512 3
```

### 2. `benchmark_gemm.py`
一个简单的 Python 启动脚本，用来：
- 编译 `gemm_baseline.cpp`
- 跑几组默认尺寸

运行：

```bash
python3 benchmark_gemm.py
```

或者指定尺寸：

```bash
python3 benchmark_gemm.py 512 512 512
```

## 后续可继续加
- blocked GEMM
- OpenMP 并行版本
- SIMD intrinsic 示例
- naive vs packed GEMM
- CPU vs GPU 对照实验
