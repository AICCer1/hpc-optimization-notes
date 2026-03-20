#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

void gemm_ikj_serial(const std::vector<float>& A,
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

void gemm_ikj_openmp(const std::vector<float>& A,
                     const std::vector<float>& B,
                     std::vector<float>& C,
                     int M,
                     int N,
                     int K) {
    std::fill(C.begin(), C.end(), 0.0f);

#pragma omp parallel for schedule(static)
    for (int i = 0; i < M; ++i) {
        for (int k = 0; k < K; ++k) {
            float a = A[i * K + k];
            for (int j = 0; j < N; ++j) {
                C[i * N + j] += a * B[k * N + j];
            }
        }
    }
}

double benchmark_serial(const std::vector<float>& A,
                        const std::vector<float>& B,
                        std::vector<float>& C,
                        int M,
                        int N,
                        int K,
                        int repeat) {
    double best_ms = 1e100;
    for (int r = 0; r < repeat; ++r) {
        auto start = std::chrono::high_resolution_clock::now();
        gemm_ikj_serial(A, B, C, M, N, K);
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        best_ms = std::min(best_ms, ms);
    }
    return best_ms;
}

double benchmark_openmp(const std::vector<float>& A,
                        const std::vector<float>& B,
                        std::vector<float>& C,
                        int M,
                        int N,
                        int K,
                        int repeat) {
    double best_ms = 1e100;
    for (int r = 0; r < repeat; ++r) {
        auto start = std::chrono::high_resolution_clock::now();
        gemm_ikj_openmp(A, B, C, M, N, K);
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

    double ms_serial = benchmark_serial(A, B, C, M, N, K, repeat);
    double gflops_serial = gflops(M, N, K, ms_serial);

    double ms_openmp = benchmark_openmp(A, B, C, M, N, K, repeat);
    double gflops_openmp = gflops(M, N, K, ms_openmp);

    std::cout << "OpenMP GEMM benchmark\n";
    std::cout << "M=" << M << ", N=" << N << ", K=" << K << ", repeat=" << repeat << "\n";
#ifdef _OPENMP
    std::cout << "OpenMP enabled, max threads=" << omp_get_max_threads() << "\n\n";
#else
    std::cout << "OpenMP not enabled at compile time\n\n";
#endif

    std::cout << std::fixed << std::setprecision(3);
    std::cout << "serial : " << ms_serial << " ms, " << gflops_serial << " GFLOPS\n";
    std::cout << "openmp : " << ms_openmp << " ms, " << gflops_openmp << " GFLOPS\n";

    return 0;
}
