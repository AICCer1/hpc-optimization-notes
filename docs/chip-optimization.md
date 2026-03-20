# Chip Optimization

## 可展开方向
- CPU pipeline 与乱序执行
- Cache blocking
- Prefetch
- SIMD 指令利用率
- GPU occupancy
- Shared memory / global memory / register pressure
- 数据布局：AoS vs SoA
- 算子 fusion

## 分析框架
遇到性能问题时，可以先问：
1. 算力瓶颈还是带宽瓶颈？
2. 热点函数在哪里？
3. cache miss 高不高？
4. 是否存在访存不连续？
5. 是否有向量化失败？
6. 是否有线程负载不均衡？
