# Glossary

- **Roofline Model**：用来分析算子/程序性能上界的模型
- **Compute-bound**：主要受算力限制
- **Memory-bound**：主要受内存带宽/访存限制
- **Blocking / Tiling**：把大问题切块，提高局部性
- **Vectorization**：利用 SIMD 并行处理多个数据
- **Occupancy**：GPU 上活跃线程块/warp 的占用情况
- **Prefetch**：提前加载未来要访问的数据
