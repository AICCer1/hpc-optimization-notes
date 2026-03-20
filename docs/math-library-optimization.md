# Math Library Optimization

## 重点对象
- BLAS
- GEMM
- FFT
- Sparse kernels
- Convolution / Tensor operators
- Reduction / Scan

## 优化常见手段
- blocking / tiling
- packing
- prefetch
- vectorization
- loop unrolling
- thread partitioning
- memory reuse
- kernel fusion

## 后续可扩展
- GEMM from naive to optimized
- BLIS / OpenBLAS / MKL 设计思路
- CUTLASS / cuBLAS 优化套路
- Triton / TVM 自动优化思路
