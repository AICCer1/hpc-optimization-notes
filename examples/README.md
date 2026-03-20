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

### 2. `gemm_blocked.cpp`
一个更进一步的 GEMM 示例，对比：
- `ikj` baseline
- blocked / tiled 版本

适合新手观察：
- blocking 是怎么写出来的
- tile size 改变可能如何影响性能

编译示例：

```bash
g++ -O3 -march=native -std=c++17 gemm_blocked.cpp -o gemm_blocked
./gemm_blocked 512 512 512 3 64 64 64
```

参数依次是：
- `M N K`
- `repeat`
- `BM BN BK`

### 3. `benchmark_gemm.py`
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

## 建议怎么用这些例子

建议按这个顺序：
1. 先跑 `gemm_baseline.cpp`
2. 看 `ijk` 和 `ikj` 差异
3. 再跑 `gemm_blocked.cpp`
4. 改 `BM BN BK` 观察变化

这样你能比较直观地体会：
- 循环顺序
- cache locality
- blocking

这些概念不是空话，是真会影响结果。

### 4. `gemm_openmp.cpp`
一个 OpenMP 版本的 GEMM 示例，对比：
- serial `ikj`
- OpenMP 并行 `ikj`

编译示例：

```bash
g++ -O3 -march=native -std=c++17 -fopenmp gemm_openmp.cpp -o gemm_openmp
./gemm_openmp 512 512 512 3
```

你也可以通过环境变量控制线程数：

```bash
OMP_NUM_THREADS=1 ./gemm_openmp 512 512 512 3
OMP_NUM_THREADS=4 ./gemm_openmp 512 512 512 3
OMP_NUM_THREADS=8 ./gemm_openmp 512 512 512 3
```

这很适合观察：
- 多线程是否真的带来加速
- 线程数增加后收益是否线性
- memory-bound 场景下线程增加为什么不一定一直赚

## 后续可继续加
- SIMD intrinsic 示例
- naive vs packed GEMM
- CPU vs GPU 对照实验
