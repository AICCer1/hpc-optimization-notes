#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <immintrin.h>
#include <iostream>
#include <random>
#include <vector>

void gemm_scalar_ikj(const std::vector<float>& A,
                     const std::vector<float>& B,
                     std::vector<float>& C,
                     int M,
                     int N,
                     int K) {
    std::fill(C.begin(), C.end(), 0.0f);
    for (int i = 0; i < M; ++i) {
        for (int k = 0; k < K; ++k) {
            float a = A[i * K + k];
            for (int j = 0; j < N; ++j) {
                C[i * N + j] += a * B[k * N + j];
            }
        }
    }
}

void gemm_avx_ikj(const std::vector<float>& A,
                  const std::vector<float>& B,
                  std::vector<float>& C,
                  int M,
                  int N,
                  int K) {
    std::fill(C.begin(), C.end(), 0.0f);

    for (int i = 0; i < M; ++i) {
        for (int k = 0; k < K; ++k) {
            __m256 a8 = _mm256_set1_ps(A[i * K + k]);
            int j = 0;
            for (; j + 8 <= N; j += 8) {
                __m256 b8 = _mm256_loadu_ps(&B[k * N + j]);
                __m256 c8 = _mm256_loadu_ps(&C[i * N + j]);
                __m256 prod = _mm256_mul_ps(a8, b8);
                c8 = _mm256_add_ps(c8, prod);
                _mm256_storeu_ps(&C[i * N + j], c8);
            }
            for (; j < N; ++j) {
                C[i * N + j] += A[i * K + k] * B[k * N + j];
            }
        }
    }
}

double benchmark_scalar(const std::vector<float>& A,
                        const std::vector<float>& B,
                        std::vector<float>& C,
                        int M,
                        int N,
                        int K,
                        int repeat) {
    double best_ms = 1e100;
    for (int r = 0; r < repeat; ++r) {
        auto start = std::chrono::high_resolution_clock::now();
        gemm_scalar_ikj(A, B, C, M, N, K);
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        best_ms = std::min(best_ms, ms);
    }
    return best_ms;
}

double benchmark_avx(const std::vector<float>& A,
                     const std::vector<float>& B,
                     std::vector<float>& C,
                     int M,
                     int N,
                     int K,
                     int repeat) {
    double best_ms = 1e100;
    for (int r = 0; r < repeat; ++r) {
        auto start = std::chrono::high_resolution_clock::now();
        gemm_avx_ikj(A, B, C, M, N, K);
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        best_ms = std::min(best_ms, ms);
    }
    return best_ms;
}

double gflops(int M, int N, int K, double ms) {
    double flops = 2.0 * M * N * K;
    return flops / 1e9 / (ms / 1e3);
}

int main(int argc, char** argv) {
    int M = 512;
    int N = 512;
    int K = 512;
    int repeat = 3;

    if (argc >= 4) {
        M = std::atoi(argv[1]);
        N = std::atoi(argv[2]);
        K = std::atoi(argv[3]);
    }
    if (argc >= 5) {
        repeat = std::atoi(argv[4]);
    }

    std::vector<float> A(M * K);
    std::vector<float> B(K * N);
    std::vector<float> C(M * N);

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    for (auto& x : A) x = dist(rng);
    for (auto& x : B) x = dist(rng);

    double ms_scalar = benchmark_scalar(A, B, C, M, N, K, repeat);
    double gflops_scalar = gflops(M, N, K, ms_scalar);

    double ms_avx = benchmark_avx(A, B, C, M, N, K, repeat);
    double gflops_avx = gflops(M, N, K, ms_avx);

    std::cout << "SIMD GEMM benchmark (AVX)\n";
    std::cout << "M=" << M << ", N=" << N << ", K=" << K << ", repeat=" << repeat << "\n\n";
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "scalar : " << ms_scalar << " ms, " << gflops_scalar << " GFLOPS\n";
    std::cout << "avx    : " << ms_avx << " ms, " << gflops_avx << " GFLOPS\n";

    return 0;
}
